// API Configuration
const API_URL = window.location.hostname === 'localhost' 
    ? 'http://localhost:5000'
    : 'https://ghost-sec-backend.herokuapp.com';

// Form handling functions
async function handleContactForm(event) {
    event.preventDefault();
    
    const formData = {
        name: document.getElementById('contact-name').value,
        email: document.getElementById('contact-email').value,
        message: document.getElementById('contact-message').value
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
            showNotification('Error sending message. Please try again.', 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error sending message. Please try again.', 'error');
    }
}

async function handleGhostSecSignup(event) {
    event.preventDefault();
    
    const formData = {
        name: document.getElementById('signup-name').value,
        email: document.getElementById('signup-email').value,
        expertise: document.getElementById('signup-expertise').value,
        interests: Array.from(document.querySelectorAll('input[name="interests"]:checked'))
            .map(cb => cb.value)
            .join(', ')
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
            showNotification('Signup successful! Welcome to GHOST Sec!', 'success');
            event.target.reset();
        } else {
            showNotification('Error during signup. Please try again.', 'error');
        }
    } catch (error) {
        console.error('Error:', error);
        showNotification('Error during signup. Please try again.', 'error');
    }
}

// Utility functions
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    // Fade in
    setTimeout(() => notification.classList.add('show'), 100);
    
    // Remove after 3 seconds
    setTimeout(() => {
        notification.classList.remove('show');
        setTimeout(() => notification.remove(), 300);
    }, 3000);
}

// Event listeners
document.addEventListener('DOMContentLoaded', () => {
    const contactForm = document.getElementById('contact-form');
    if (contactForm) {
        contactForm.addEventListener('submit', handleContactForm);
    }
    
    const signupForm = document.getElementById('ghost-sec-signup');
    if (signupForm) {
        signupForm.addEventListener('submit', handleGhostSecSignup);
    }
});
