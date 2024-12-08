use std::fs::{self, File};
use std::io::{self, Read};
use std::path::Path;
use sha2::{Sha256, Digest};
use anyhow::{Result, Context};
use log::{info, warn};

pub fn check_integrity(path: &str, baseline: Option<&str>) -> Result<()> {
    let path = Path::new(path);
    if path.is_dir() {
        check_directory(path, baseline)?;
    } else {
        let hash = calculate_file_hash(path)?;
        match baseline {
            Some(baseline_file) => {
                compare_with_baseline(&hash, baseline_file)?;
            }
            None => {
                info!("SHA-256 hash for {}: {}", path.display(), hash);
            }
        }
    }
    Ok(())
}

fn calculate_file_hash<P: AsRef<Path>>(path: P) -> Result<String> {
    let mut file = File::open(&path).context("Failed to open file")?;
    let mut hasher = Sha256::new();
    let mut buffer = [0; 1024];

    loop {
        let bytes_read = file.read(&mut buffer)?;
        if bytes_read == 0 {
            break;
        }
        hasher.update(&buffer[..bytes_read]);
    }

    Ok(hex::encode(hasher.finalize()))
}

fn check_directory(dir: &Path, baseline: Option<&str>) -> Result<()> {
    for entry in fs::read_dir(dir)? {
        let entry = entry?;
        let path = entry.path();
        
        if path.is_dir() {
            check_directory(&path, baseline)?;
        } else {
            let hash = calculate_file_hash(&path)?;
            info!("File: {}, SHA-256: {}", path.display(), hash);
        }
    }
    Ok(())
}

fn compare_with_baseline(current_hash: &str, baseline_file: &str) -> Result<()> {
    let baseline_content = fs::read_to_string(baseline_file)
        .context("Failed to read baseline file")?;
    
    let baseline_hash = baseline_content.trim();
    
    if current_hash == baseline_hash {
        info!("File integrity verified - hash matches baseline");
    } else {
        warn!("File integrity check failed - hash does not match baseline");
        warn!("Current:  {}", current_hash);
        warn!("Baseline: {}", baseline_hash);
    }
    
    Ok(())
}
