from flask import Flask, request, jsonify
from flask_cors import CORS
from flask_sqlalchemy import SQLAlchemy
from flask_mail import Mail, Message
from datetime import datetime
import os
from dotenv import load_dotenv

# Load environment variables
load_dotenv()

app = Flask(__name__)
CORS(app)

# Database configuration
app.config['SQLALCHEMY_DATABASE_URI'] = os.getenv('DATABASE_URL', 'sqlite:///messages.db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# Email configuration
app.config['MAIL_SERVER'] = 'smtp.gmail.com'
app.config['MAIL_PORT'] = 587
app.config['MAIL_USE_TLS'] = True
app.config['MAIL_USERNAME'] = os.getenv('EMAIL_USER')
app.config['MAIL_PASSWORD'] = os.getenv('EMAIL_PASS')
app.config['MAIL_DEFAULT_SENDER'] = os.getenv('EMAIL_USER')

db = SQLAlchemy(app)
mail = Mail(app)

# Database Models
class ContactMessage(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    email = db.Column(db.String(120), nullable=False)
    message = db.Column(db.Text, nullable=False)
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)
    
class GhostSecSignup(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    email = db.Column(db.String(120), nullable=False)
    expertise = db.Column(db.String(50), nullable=False)
    interests = db.Column(db.Text, nullable=False)
    timestamp = db.Column(db.DateTime, default=datetime.utcnow)

def send_notification_email(subject, body):
    """Send notification email to admin"""
    try:
        msg = Message(
            subject=subject,
            recipients=[os.getenv('ADMIN_EMAIL', 'anonymous23261@icloud.com')],
            body=body
        )
        mail.send(msg)
    except Exception as e:
        print(f"Error sending email: {e}")

@app.route('/api/contact', methods=['POST'])
def contact():
    try:
        data = request.json
        
        # Create new message
        message = ContactMessage(
            name=data['name'],
            email=data['email'],
            message=data['message']
        )
        db.session.add(message)
        db.session.commit()
        
        # Send notification email
        email_body = f"""
        New Contact Message:
        From: {data['name']} ({data['email']})
        Message: {data['message']}
        """
        send_notification_email("New Contact Form Submission", email_body)
        
        return jsonify({"status": "success", "message": "Message sent successfully"}), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/signup', methods=['POST'])
def signup():
    try:
        data = request.json
        
        # Create new signup
        signup = GhostSecSignup(
            name=data['name'],
            email=data['email'],
            expertise=data['expertise'],
            interests=data['interests']
        )
        db.session.add(signup)
        db.session.commit()
        
        # Send notification email
        email_body = f"""
        New GHOST Sec Signup:
        Name: {data['name']}
        Email: {data['email']}
        Expertise Level: {data['expertise']}
        Interests: {data['interests']}
        """
        send_notification_email("New GHOST Sec Signup", email_body)
        
        return jsonify({"status": "success", "message": "Signup successful"}), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/messages', methods=['GET'])
def get_messages():
    """Get all messages (protected endpoint)"""
    try:
        messages = ContactMessage.query.order_by(ContactMessage.timestamp.desc()).all()
        return jsonify([{
            'id': m.id,
            'name': m.name,
            'email': m.email,
            'message': m.message,
            'timestamp': m.timestamp.isoformat()
        } for m in messages]), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/api/signups', methods=['GET'])
def get_signups():
    """Get all signups (protected endpoint)"""
    try:
        signups = GhostSecSignup.query.order_by(GhostSecSignup.timestamp.desc()).all()
        return jsonify([{
            'id': s.id,
            'name': s.name,
            'email': s.email,
            'expertise': s.expertise,
            'interests': s.interests,
            'timestamp': s.timestamp.isoformat()
        } for s in signups]), 200
    except Exception as e:
        return jsonify({"status": "error", "message": str(e)}), 500

if __name__ == '__main__':
    with app.app_context():
        db.create_all()
    app.run(debug=True, port=5000)
