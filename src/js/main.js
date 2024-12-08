// Wait for DOM to be fully loaded before initializing
document.addEventListener('DOMContentLoaded', function() {
    createMatrixRain();
    
    // Add glitch effect to matrix effect elements
    document.querySelectorAll('.matrix-effect').forEach(addGlitchEffect);
    
    // Add hover effects to cards
    const cards = document.querySelectorAll('.project-card, .lab-card, .tool-card');
    cards.forEach(card => {
        card.addEventListener('mouseover', () => {
            card.style.transform = 'translateY(-5px)';
            card.style.boxShadow = '0 5px 15px rgba(0, 255, 0, 0.2)';
        });
        card.addEventListener('mouseout', () => {
            card.style.transform = 'translateY(0)';
            card.style.boxShadow = 'none';
        });
    });

    // Smooth scroll
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

    // Form handling
    const form = document.getElementById('contactForm');
    if (form) {
        form.addEventListener('submit', function(e) {
            // Formspree will handle the submission
            // We just need to validate and show a loading state
            const name = document.getElementById('name').value.trim();
            const email = document.getElementById('email').value.trim();
            const message = document.getElementById('message').value.trim();

            if (!name || !email || !message) {
                e.preventDefault();
                alert('Please fill in all fields');
                return;
            }

            // Add loading state
            const submitButton = form.querySelector('button[type="submit"]');
            submitButton.disabled = true;
            submitButton.textContent = 'Transmitting...';

            // Formspree will handle the rest and redirect to thanks.html
        });
    }

    // Matrix rain effect
    function createMatrixRain() {
        const canvas = document.createElement('canvas');
        canvas.style.position = 'fixed';
        canvas.style.top = '0';
        canvas.style.left = '0';
        canvas.style.width = '100%';
        canvas.style.height = '100%';
        canvas.style.zIndex = '-1';
        canvas.style.opacity = '0.05';
        document.body.appendChild(canvas);

        const ctx = canvas.getContext('2d');
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;

        const characters = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%^&*()';
        const fontSize = 10;
        const columns = canvas.width / fontSize;
        const drops = [];

        for (let i = 0; i < columns; i++) {
            drops[i] = 1;
        }

        function draw() {
            ctx.fillStyle = 'rgba(0, 0, 0, 0.05)';
            ctx.fillRect(0, 0, canvas.width, canvas.height);
            ctx.fillStyle = '#0F0';
            ctx.font = fontSize + 'px monospace';

            for (let i = 0; i < drops.length; i++) {
                const text = characters[Math.floor(Math.random() * characters.length)];
                ctx.fillText(text, i * fontSize, drops[i] * fontSize);
                if (drops[i] * fontSize > canvas.height && Math.random() > 0.975) {
                    drops[i] = 0;
                }
                drops[i]++;
            }
        }

        window.addEventListener('resize', () => {
            canvas.width = window.innerWidth;
            canvas.height = window.innerHeight;
        });

        setInterval(draw, 35);
    }

    // Glitch text effect
    function addGlitchEffect(element) {
        element.addEventListener('mouseover', () => {
            let iterations = 0;
            const originalText = element.textContent;
            const glitchInterval = setInterval(() => {
                element.textContent = originalText
                    .split('')
                    .map((char, index) => {
                        if (index < iterations) {
                            return originalText[index];
                        }
                        return characters[Math.floor(Math.random() * characters.length)];
                    })
                    .join('');
                iterations += 1/3;
                if (iterations >= originalText.length) {
                    clearInterval(glitchInterval);
                    element.textContent = originalText;
                }
            }, 30);
        });
    }
});
