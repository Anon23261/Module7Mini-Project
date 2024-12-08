// API Configuration
const API_URL = window.location.hostname === 'localhost' 
    ? 'http://localhost:5000'
    : 'https://api.ghostsec.dev';

// Form submission handler
function handleFormSubmission(event) {
    event.preventDefault();
    
    const form = event.target;
    const formData = new FormData(form);
    const data = {};
    
    for (let [key, value] of formData.entries()) {
        data[key] = value;
    }
    
    // Validate form data
    if (!validateForm(data)) {
        return;
    }
    
    // Submit form data
    submitForm(data)
        .then(response => {
            showNotification('Message sent successfully!', 'success');
            form.reset();
        })
        .catch(error => {
            showNotification('Error sending message. Please try again.', 'error');
            console.error('Form submission error:', error);
        });
}

// Form validation
function validateForm(data) {
    if (!data.name || data.name.trim() === '') {
        showNotification('Please enter your name', 'error');
        return false;
    }
    
    if (!data.email || !isValidEmail(data.email)) {
        showNotification('Please enter a valid email address', 'error');
        return false;
    }
    
    if (!data.message || data.message.trim() === '') {
        showNotification('Please enter a message', 'error');
        return false;
    }
    
    return true;
}

// Email validation
function isValidEmail(email) {
    const emailRegex = /^[^\s@]+@[^\s@]+\.[^\s@]+$/;
    return emailRegex.test(email);
}

// Form submission to backend
async function submitForm(data) {
    const response = await fetch(`${API_URL}/api/contact`, {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(data)
    });
    
    if (!response.ok) {
        throw new Error('Network response was not ok');
    }
    
    return response.json();
}

// Notification system
function showNotification(message, type = 'info') {
    const notification = document.createElement('div');
    notification.className = `notification ${type}`;
    notification.textContent = message;
    
    document.body.appendChild(notification);
    
    // Remove notification after 5 seconds
    setTimeout(() => {
        notification.remove();
    }, 5000);
}

// Smooth scrolling for navigation links
document.querySelectorAll('a[href^="#"]').forEach(anchor => {
    anchor.addEventListener('click', function(e) {
        e.preventDefault();
        const target = document.querySelector(this.getAttribute('href'));
        if (target) {
            target.scrollIntoView({
                behavior: 'smooth',
                block: 'start'
            });
        }
    });
});

// Matrix rain effect
class MatrixRain {
    constructor() {
        this.canvas = document.createElement('canvas');
        this.ctx = this.canvas.getContext('2d');
        document.body.appendChild(this.canvas);
        
        this.canvas.style.position = 'fixed';
        this.canvas.style.top = '0';
        this.canvas.style.left = '0';
        this.canvas.style.zIndex = '-1';
        this.canvas.style.opacity = '0.1';
        
        this.resizeCanvas();
        window.addEventListener('resize', () => this.resizeCanvas());
        
        this.characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()';
        this.drops = [];
        
        this.initDrops();
        this.animate();
    }
    
    resizeCanvas() {
        this.canvas.width = window.innerWidth;
        this.canvas.height = window.innerHeight;
        this.fontSize = 14;
        this.columns = Math.floor(this.canvas.width / this.fontSize);
        this.initDrops();
    }
    
    initDrops() {
        this.drops = [];
        for(let i = 0; i < this.columns; i++) {
            this.drops[i] = Math.random() * -100;
        }
    }
    
    animate() {
        this.ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        
        this.ctx.fillStyle = '#0F0';
        this.ctx.font = this.fontSize + 'px monospace';
        
        for(let i = 0; i < this.drops.length; i++) {
            const char = this.characters[Math.floor(Math.random() * this.characters.length)];
            this.ctx.fillText(char, i * this.fontSize, this.drops[i] * this.fontSize);
            
            if(this.drops[i] * this.fontSize > this.canvas.height && Math.random() > 0.975) {
                this.drops[i] = 0;
            }
            this.drops[i]++;
        }
        requestAnimationFrame(() => this.animate());
    }
}

// Terminal typing effect
class TerminalTyping {
    constructor(element, text, speed = 50) {
        this.element = element;
        this.text = text;
        this.speed = speed;
        this.index = 0;
        this.type();
    }
    
    type() {
        if (this.index < this.text.length) {
            this.element.textContent += this.text.charAt(this.index);
            this.index++;
            setTimeout(() => this.type(), this.speed);
        }
    }
}

// Initialize event listeners when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    // Initialize Matrix rain
    new MatrixRain();
    
    // Remove loading screen
    const loadingScreen = document.querySelector('.loading');
    if (loadingScreen) {
        setTimeout(() => {
            loadingScreen.style.opacity = '0';
            setTimeout(() => {
                loadingScreen.remove();
            }, 500);
        }, 1500);
    }
    
    // Add form handlers
    const forms = document.querySelectorAll('form');
    forms.forEach(form => {
        form.addEventListener('submit', handleFormSubmission);
    });
    
    // Add hover effect to navigation
    const navLinks = document.querySelectorAll('nav a');
    navLinks.forEach(link => {
        link.addEventListener('mouseenter', () => {
            const glitchSound = new Audio('assets/sounds/glitch.mp3');
            glitchSound.volume = 0.1;
            glitchSound.play().catch(() => {});
        });
    });
    
    // Initialize terminal typing effects
    const terminalElements = document.querySelectorAll('.terminal-effect');
    terminalElements.forEach(element => {
        const originalText = element.textContent;
        element.textContent = '';
        new TerminalTyping(element, originalText);
    });
});

// Add glitch effect on scroll
let lastScrollPosition = window.pageYOffset;
window.addEventListener('scroll', () => {
    const currentScrollPosition = window.pageYOffset;
    const header = document.querySelector('header');
    
    if (Math.abs(currentScrollPosition - lastScrollPosition) > 50) {
        header.style.transform = `glitch(${Math.random() * 10}px)`;
        setTimeout(() => {
            header.style.transform = 'none';
        }, 100);
        lastScrollPosition = currentScrollPosition;
    }
});
