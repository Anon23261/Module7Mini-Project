-- Enhanced Database Forensics Tool
-- This script creates tables and functions for comprehensive database security monitoring

-- Enable necessary extensions
CREATE EXTENSION IF NOT EXISTS pgcrypto;
CREATE EXTENSION IF NOT EXISTS pg_stat_statements;

-- Create schema for forensics
CREATE SCHEMA IF NOT EXISTS db_forensics;

-- Audit log table
CREATE TABLE IF NOT EXISTS db_forensics.audit_log (
    id BIGSERIAL PRIMARY KEY,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    user_name TEXT,
    database_name TEXT,
    schema_name TEXT,
    table_name TEXT,
    operation TEXT,
    query TEXT,
    old_data JSONB,
    new_data JSONB,
    client_ip INET,
    application_name TEXT,
    session_id TEXT,
    transaction_id BIGINT,
    query_duration INTERVAL
);

-- Suspicious activity patterns
CREATE TABLE IF NOT EXISTS db_forensics.suspicious_patterns (
    id SERIAL PRIMARY KEY,
    pattern_name TEXT,
    pattern_regex TEXT,
    severity TEXT CHECK (severity IN ('LOW', 'MEDIUM', 'HIGH', 'CRITICAL')),
    description TEXT,
    remediation TEXT,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
);

-- Security alerts
CREATE TABLE IF NOT EXISTS db_forensics.security_alerts (
    id BIGSERIAL PRIMARY KEY,
    timestamp TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    alert_type TEXT,
    severity TEXT CHECK (severity IN ('LOW', 'MEDIUM', 'HIGH', 'CRITICAL')),
    description TEXT,
    affected_objects TEXT[],
    user_name TEXT,
    client_ip INET,
    query TEXT,
    remediation_suggested TEXT,
    status TEXT DEFAULT 'OPEN'
);

-- User activity statistics
CREATE TABLE IF NOT EXISTS db_forensics.user_activity_stats (
    id BIGSERIAL PRIMARY KEY,
    user_name TEXT,
    start_time TIMESTAMP WITH TIME ZONE,
    end_time TIMESTAMP WITH TIME ZONE,
    total_queries INTEGER,
    failed_queries INTEGER,
    data_modified BIGINT,
    suspicious_actions INTEGER,
    avg_query_duration INTERVAL,
    ip_addresses INET[]
);

-- Insert common suspicious patterns
INSERT INTO db_forensics.suspicious_patterns (pattern_name, pattern_regex, severity, description, remediation) 
VALUES 
    ('SQL Injection Attempt', '(''\s*OR\s*''1''=''1|UNION\s+SELECT|WAITFOR\s+DELAY|SELECT\s+@@VERSION)', 'HIGH', 
     'Possible SQL injection attempt detected', 'Implement prepared statements and input validation'),
    ('Mass Data Access', '(SELECT\s+\*\s+FROM\s+\w+\s+WHERE\s+1\s*=\s*1)', 'HIGH',
     'Attempting to retrieve all records from a table', 'Implement row-level security and limit query results'),
    ('Privilege Escalation', '(ALTER\s+USER|GRANT\s+.*\s+TO|CREATE\s+USER)', 'CRITICAL',
     'Attempt to modify user privileges or create new users', 'Review security policies and implement least privilege principle'),
    ('Data Export Attempt', '(COPY|INTO\s+OUTFILE|DUMP)', 'HIGH',
     'Attempting to export data from the database', 'Restrict file system access and monitor data exports'),
    ('Schema Modification', '(DROP\s+TABLE|ALTER\s+TABLE|TRUNCATE\s+TABLE)', 'HIGH',
     'Attempting to modify database schema', 'Implement change management procedures and backup policies');

-- Function to analyze query for suspicious patterns
CREATE OR REPLACE FUNCTION db_forensics.analyze_query(
    p_query TEXT,
    p_user_name TEXT,
    p_client_ip INET
) RETURNS TABLE (
    is_suspicious BOOLEAN,
    matched_patterns TEXT[],
    severity TEXT,
    recommendations TEXT[]
) AS $$
DECLARE
    v_pattern RECORD;
    v_matched_patterns TEXT[] := ARRAY[]::TEXT[];
    v_recommendations TEXT[] := ARRAY[]::TEXT[];
    v_max_severity TEXT := 'LOW';
BEGIN
    FOR v_pattern IN SELECT * FROM db_forensics.suspicious_patterns
    LOOP
        IF p_query ~* v_pattern.pattern_regex THEN
            v_matched_patterns := array_append(v_matched_patterns, v_pattern.pattern_name);
            v_recommendations := array_append(v_recommendations, v_pattern.remediation);
            
            -- Update max severity
            IF v_pattern.severity = 'CRITICAL' THEN
                v_max_severity := 'CRITICAL';
            ELSIF v_pattern.severity = 'HIGH' AND v_max_severity NOT IN ('CRITICAL') THEN
                v_max_severity := 'HIGH';
            ELSIF v_pattern.severity = 'MEDIUM' AND v_max_severity NOT IN ('CRITICAL', 'HIGH') THEN
                v_max_severity := 'MEDIUM';
            END IF;
        END IF;
    END LOOP;

    -- Create security alert if suspicious patterns found
    IF array_length(v_matched_patterns, 1) > 0 THEN
        INSERT INTO db_forensics.security_alerts (
            alert_type,
            severity,
            description,
            affected_objects,
            user_name,
            client_ip,
            query,
            remediation_suggested
        ) VALUES (
            'SUSPICIOUS_QUERY',
            v_max_severity,
            'Suspicious query patterns detected: ' || array_to_string(v_matched_patterns, ', '),
            ARRAY[current_database()],
            p_user_name,
            p_client_ip,
            p_query,
            array_to_string(v_recommendations, '; ')
        );
    END IF;

    RETURN QUERY SELECT 
        array_length(v_matched_patterns, 1) > 0,
        v_matched_patterns,
        v_max_severity,
        v_recommendations;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Function to log database activity
CREATE OR REPLACE FUNCTION db_forensics.log_activity() RETURNS trigger AS $$
DECLARE
    v_old_data JSONB := NULL;
    v_new_data JSONB := NULL;
BEGIN
    -- Capture old and new data for DML operations
    IF (TG_OP = 'DELETE') THEN
        v_old_data := to_jsonb(OLD);
    ELSIF (TG_OP = 'UPDATE') THEN
        v_old_data := to_jsonb(OLD);
        v_new_data := to_jsonb(NEW);
    ELSIF (TG_OP = 'INSERT') THEN
        v_new_data := to_jsonb(NEW);
    END IF;

    -- Log the activity
    INSERT INTO db_forensics.audit_log (
        user_name,
        database_name,
        schema_name,
        table_name,
        operation,
        query,
        old_data,
        new_data,
        client_ip,
        application_name,
        session_id,
        transaction_id,
        query_duration
    ) VALUES (
        session_user,
        current_database(),
        TG_TABLE_SCHEMA,
        TG_TABLE_NAME,
        TG_OP,
        current_query(),
        v_old_data,
        v_new_data,
        inet_client_addr(),
        application_name(),
        pg_backend_pid()::TEXT,
        txid_current(),
        clock_timestamp() - transaction_timestamp()
    );

    RETURN NULL;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Function to create audit triggers for a table
CREATE OR REPLACE FUNCTION db_forensics.enable_table_audit(
    p_schema TEXT,
    p_table TEXT
) RETURNS VOID AS $$
DECLARE
    v_trigger_name TEXT;
BEGIN
    v_trigger_name := 'audit_trigger_' || p_table;
    
    EXECUTE format(
        'CREATE TRIGGER %I
         AFTER INSERT OR UPDATE OR DELETE ON %I.%I
         FOR EACH ROW EXECUTE FUNCTION db_forensics.log_activity()',
        v_trigger_name, p_schema, p_table
    );
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Function to analyze user activity patterns
CREATE OR REPLACE FUNCTION db_forensics.analyze_user_activity(
    p_start_time TIMESTAMP WITH TIME ZONE,
    p_end_time TIMESTAMP WITH TIME ZONE
) RETURNS TABLE (
    user_name TEXT,
    total_queries BIGINT,
    suspicious_queries BIGINT,
    failed_operations BIGINT,
    avg_query_duration INTERVAL,
    distinct_tables INTEGER,
    data_modification_ops BIGINT,
    high_risk_operations TEXT[]
) AS $$
BEGIN
    RETURN QUERY
    WITH activity_summary AS (
        SELECT 
            al.user_name,
            count(*) as total_queries,
            count(*) FILTER (WHERE sa.id IS NOT NULL) as suspicious_queries,
            count(*) FILTER (WHERE al.operation NOT IN ('SELECT', 'INSERT', 'UPDATE', 'DELETE')) as high_risk_ops,
            avg(al.query_duration) as avg_duration,
            count(DISTINCT al.table_name) as distinct_tables,
            count(*) FILTER (WHERE al.operation IN ('INSERT', 'UPDATE', 'DELETE')) as data_mods
        FROM db_forensics.audit_log al
        LEFT JOIN db_forensics.security_alerts sa 
            ON al.user_name = sa.user_name 
            AND al.query = sa.query
        WHERE al.timestamp BETWEEN p_start_time AND p_end_time
        GROUP BY al.user_name
    )
    SELECT 
        a.user_name,
        a.total_queries,
        a.suspicious_queries,
        a.high_risk_ops,
        a.avg_duration,
        a.distinct_tables,
        a.data_mods,
        array_agg(DISTINCT al.operation) FILTER (WHERE al.operation NOT IN ('SELECT', 'INSERT', 'UPDATE', 'DELETE'))
    FROM activity_summary a
    LEFT JOIN db_forensics.audit_log al ON a.user_name = al.user_name
    GROUP BY 
        a.user_name, 
        a.total_queries, 
        a.suspicious_queries, 
        a.high_risk_ops,
        a.avg_duration,
        a.distinct_tables,
        a.data_mods;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Function to generate security report
CREATE OR REPLACE FUNCTION db_forensics.generate_security_report(
    p_start_time TIMESTAMP WITH TIME ZONE,
    p_end_time TIMESTAMP WITH TIME ZONE
) RETURNS TABLE (
    report_section TEXT,
    finding_type TEXT,
    severity TEXT,
    description TEXT,
    affected_objects TEXT[],
    recommendation TEXT
) AS $$
BEGIN
    -- High-risk operations
    INSERT INTO db_forensics.security_alerts (
        alert_type,
        severity,
        description,
        affected_objects,
        user_name,
        remediation_suggested
    )
    SELECT 
        'HIGH_RISK_OPERATION',
        'HIGH',
        'High-risk database operation detected: ' || operation,
        ARRAY[table_name],
        user_name,
        'Review and implement least privilege principle'
    FROM db_forensics.audit_log
    WHERE timestamp BETWEEN p_start_time AND p_end_time
    AND operation NOT IN ('SELECT', 'INSERT', 'UPDATE', 'DELETE');

    -- Suspicious access patterns
    INSERT INTO db_forensics.security_alerts (
        alert_type,
        severity,
        description,
        affected_objects,
        user_name,
        remediation_suggested
    )
    SELECT 
        'SUSPICIOUS_ACCESS_PATTERN',
        'MEDIUM',
        'Unusual data access pattern detected',
        ARRAY[table_name],
        user_name,
        'Review access patterns and implement rate limiting'
    FROM (
        SELECT 
            user_name,
            table_name,
            count(*) as access_count
        FROM db_forensics.audit_log
        WHERE timestamp BETWEEN p_start_time AND p_end_time
        GROUP BY user_name, table_name
        HAVING count(*) > 1000
    ) suspicious_access;

    -- Return comprehensive report
    RETURN QUERY
    SELECT
        CASE
            WHEN alert_type = 'SUSPICIOUS_QUERY' THEN 'Suspicious Queries'
            WHEN alert_type = 'HIGH_RISK_OPERATION' THEN 'High-risk Operations'
            WHEN alert_type = 'SUSPICIOUS_ACCESS_PATTERN' THEN 'Suspicious Access Patterns'
            ELSE 'Other Security Concerns'
        END as report_section,
        alert_type as finding_type,
        severity,
        description,
        affected_objects,
        remediation_suggested as recommendation
    FROM db_forensics.security_alerts
    WHERE timestamp BETWEEN p_start_time AND p_end_time
    ORDER BY 
        CASE severity
            WHEN 'CRITICAL' THEN 1
            WHEN 'HIGH' THEN 2
            WHEN 'MEDIUM' THEN 3
            WHEN 'LOW' THEN 4
            ELSE 5
        END,
        timestamp DESC;
END;
$$ LANGUAGE plpgsql SECURITY DEFINER;

-- Create indexes for better performance
CREATE INDEX IF NOT EXISTS idx_audit_log_timestamp ON db_forensics.audit_log(timestamp);
CREATE INDEX IF NOT EXISTS idx_audit_log_user ON db_forensics.audit_log(user_name);
CREATE INDEX IF NOT EXISTS idx_audit_log_operation ON db_forensics.audit_log(operation);
CREATE INDEX IF NOT EXISTS idx_security_alerts_timestamp ON db_forensics.security_alerts(timestamp);
CREATE INDEX IF NOT EXISTS idx_security_alerts_severity ON db_forensics.security_alerts(severity);

-- Example usage:
-- Enable auditing for a table:
-- SELECT db_forensics.enable_table_audit('public', 'users');

-- Analyze a specific query:
-- SELECT * FROM db_forensics.analyze_query('SELECT * FROM users WHERE 1=1', 'current_user', '192.168.1.1'::inet);

-- Generate security report:
-- SELECT * FROM db_forensics.generate_security_report(
--     (CURRENT_TIMESTAMP - INTERVAL '24 hours'),
--     CURRENT_TIMESTAMP
-- );
