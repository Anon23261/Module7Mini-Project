// API Configuration
const API_URL = window.location.hostname === 'localhost' 
    ? 'http://localhost:5000'
    : 'https://api.ghostsec.dev';

// Form handling functions
async function handleContactForm(event) {
    event.preventDefault();
    
    const formData = {
        name: event.target.querySelector('[name="contact-name"]').value,
        email: event.target.querySelector('[name="contact-email"]').value,
        message: event.target.querySelector('[name="contact-message"]').value
    };
    
    try {
        const response = await fetch(`${API_URL}/api/contact`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showNotification('Message sent successfully!', 'success');
            event.target.reset();
        } else {
            showNotification(data.message || 'Error sending message. Please try again.', 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error sending message. Please try again.', 'error');
    }
}

async function handleGhostSecSignup(event) {
    event.preventDefault();
    
    const formData = {
        name: event.target.querySelector('[name="name"]').value,
        email: event.target.querySelector('[name="email"]').value,
        interests: Array.from(event.target.querySelector('[name="interests"]').selectedOptions)
            .map(option => option.value),
        message: event.target.querySelector('[name="message"]').value
    };
    
    try {
        const response = await fetch(`${API_URL}/api/signup`, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(formData)
        });
        
        const data = await response.json();
        
        if (response.ok) {
            showNotification('Application submitted successfully!', 'success');
            event.target.reset();
        } else {
            showNotification(data.message || 'Error submitting application. Please try again.', 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error submitting application. Please try again.', 'error');
    }
}

// Utility functions
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    // Trigger animation
    setTimeout(() => notification.classList.add('show'), 100);
    
    // Remove notification after 5 seconds
    setTimeout(() => {
        notification.classList.remove('show');
        setTimeout(() => notification.remove(), 300);
    }, 5000);
}

// Event listeners
document.addEventListener('DOMContentLoaded', () => {
    const contactForm = document.getElementById('contact-form');
    const signupForm = document.getElementById('ghost-sec-signup');
    
    if (contactForm) {
        contactForm.addEventListener('submit', handleContactForm);
    }
    
    if (signupForm) {
        signupForm.addEventListener('submit', handleGhostSecSignup);
    }
});
