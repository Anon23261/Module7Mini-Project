// Matrix Rain Effect
class MatrixRain {
    constructor() {
        this.canvas = document.createElement('canvas');
        this.ctx = this.canvas.getContext('2d');
        this.characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()';
        this.fontSize = 14;
        this.columns = 0;
        this.drops = [];
        
        this.initialize();
        window.addEventListener('resize', () => this.initialize());
    }

    initialize() {
        const container = document.querySelector('.cyber-grid');
        if (!container) return;
        
        this.canvas.width = container.offsetWidth;
        this.canvas.height = container.offsetHeight;
        container.appendChild(this.canvas);
        
        this.columns = Math.floor(this.canvas.width / this.fontSize);
        this.drops = Array(this.columns).fill(1);
        
        this.ctx.fillStyle = '#0F0';
        this.ctx.font = `${this.fontSize}px monospace`;
    }

    draw() {
        this.ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);

        this.ctx.fillStyle = '#0F0';
        for (let i = 0; i < this.drops.length; i++) {
            const text = this.characters[Math.floor(Math.random() * this.characters.length)];
            this.ctx.fillText(text, i * this.fontSize, this.drops[i] * this.fontSize);
            
            if (this.drops[i] * this.fontSize > this.canvas.height && Math.random() > 0.975) {
                this.drops[i] = 0;
            }
            this.drops[i]++;
        }
    }

    animate() {
        this.draw();
        requestAnimationFrame(() => this.animate());
    }
}

// Form Handling
class ContactForm {
    constructor() {
        this.form = document.getElementById('contactForm');
        this.setupListeners();
    }

    setupListeners() {
        if (this.form) {
            this.form.addEventListener('submit', (e) => this.handleSubmit(e));
        }
    }

    async handleSubmit(e) {
        e.preventDefault();
        
        const formData = new FormData(this.form);
        const data = Object.fromEntries(formData.entries());
        
        // Simulate encryption animation
        await this.simulateEncryption();
        
        // Clear form
        this.form.reset();
        
        // Show success message
        alert('Message encrypted and sent securely!');
    }

    async simulateEncryption() {
        const btn = this.form.querySelector('button[type="submit"]');
        const originalText = btn.textContent;
        
        btn.disabled = true;
        btn.textContent = 'Encrypting...';
        
        await new Promise(resolve => setTimeout(resolve, 1500));
        
        btn.textContent = 'Sending...';
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        btn.textContent = originalText;
        btn.disabled = false;
    }
}

// Mobile Navigation
class MobileNav {
    constructor() {
        this.burger = document.querySelector('.burger');
        this.nav = document.querySelector('.nav-links');
        this.navLinks = document.querySelectorAll('.nav-links li');
        this.setupListeners();
    }

    setupListeners() {
        if (this.burger) {
            this.burger.addEventListener('click', () => this.toggleNav());
        }

        // Close mobile menu when clicking a link
        this.navLinks.forEach(link => {
            link.addEventListener('click', () => {
                if (this.nav.classList.contains('nav-active')) {
                    this.toggleNav();
                }
            });
        });
    }

    toggleNav() {
        // Toggle Navigation
        this.nav.classList.toggle('nav-active');
        
        // Animate Links
        this.navLinks.forEach((link, index) => {
            if (link.style.animation) {
                link.style.animation = '';
            } else {
                link.style.animation = `navLinkFade 0.5s ease forwards ${index / 7 + 0.3}s`;
            }
        });
        
        // Burger Animation
        this.burger.classList.toggle('toggle');
    }
}

// GHOST Sec Sign Up Form
class GhostSecSignup {
    constructor() {
        this.form = document.getElementById('ghostSecSignupForm');
        this.setupListeners();
    }

    setupListeners() {
        if (this.form) {
            this.form.addEventListener('submit', (e) => this.handleSubmit(e));
        }
    }

    async handleSubmit(e) {
        e.preventDefault();
        
        const formData = new FormData(this.form);
        const data = Object.fromEntries(formData.entries());
        
        // Simulate secure submission
        await this.simulateSecureSubmission();
        
        // Store in localStorage (for demo purposes)
        this.storeSignup(data);
        
        // Clear form
        this.form.reset();
        
        // Show success message
        this.showSuccessMessage();
    }

    async simulateSecureSubmission() {
        const btn = this.form.querySelector('button[type="submit"]');
        const originalText = btn.textContent;
        
        btn.disabled = true;
        
        // Simulate encryption
        btn.textContent = 'Encrypting Data...';
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        // Simulate secure transmission
        btn.textContent = 'Establishing Secure Channel...';
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        // Simulate processing
        btn.textContent = 'Processing Application...';
        await new Promise(resolve => setTimeout(resolve, 1000));
        
        btn.textContent = originalText;
        btn.disabled = false;
    }

    storeSignup(data) {
        // Get existing signups or initialize empty array
        const signups = JSON.parse(localStorage.getItem('ghostSecSignups') || '[]');
        
        // Add timestamp
        data.timestamp = new Date().toISOString();
        
        // Add to array
        signups.push(data);
        
        // Store back in localStorage
        localStorage.setItem('ghostSecSignups', JSON.stringify(signups));
    }

    showSuccessMessage() {
        // Create success message if it doesn't exist
        let successMsg = document.querySelector('.signup-success');
        if (!successMsg) {
            successMsg = document.createElement('div');
            successMsg.className = 'signup-success';
            successMsg.innerHTML = `
                <h4>Welcome to GHOST Sec!</h4>
                <p>Your application has been successfully encrypted and submitted. You will receive further instructions via email.</p>
                <p>Remember: Security through obscurity is no security at all.</p>
            `;
            this.form.parentNode.appendChild(successMsg);
        }
        
        // Show the message
        successMsg.classList.add('show');
        
        // Hide after 5 seconds
        setTimeout(() => {
            successMsg.classList.remove('show');
        }, 5000);
    }
}

// Initialize everything when DOM is loaded
document.addEventListener('DOMContentLoaded', () => {
    // Initialize Matrix Rain
    const matrixRain = new MatrixRain();
    matrixRain.animate();

    // Initialize Contact Form
    new ContactForm();

    // Initialize Mobile Navigation
    new MobileNav();

    // Initialize GHOST Sec Signup
    new GhostSecSignup();

    // Initialize scroll animations
    window.addEventListener('scroll', () => {
        const sections = document.querySelectorAll('section');
        sections.forEach(section => {
            const sectionTop = section.getBoundingClientRect().top;
            const triggerPoint = window.innerHeight * 0.8;
            
            if (sectionTop < triggerPoint) {
                section.classList.add('visible');
            }
        });
    });
});
