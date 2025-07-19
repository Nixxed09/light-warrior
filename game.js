// Phoenix: Light Warrior - Wave-based Survival Game
class Game {
    constructor() {
        this.canvas = document.getElementById('gameCanvas');
        this.ctx = this.canvas.getContext('2d');
        this.width = this.canvas.width;
        this.height = this.canvas.height;
        
        this.gameState = 'menu'; // menu, playing, paused, gameOver
        this.score = 0;
        this.wave = 1;
        this.health = 100;
        this.maxHealth = 100;
        
        this.player = new Player(this.width / 2, this.height / 2);
        this.enemies = [];
        this.projectiles = [];
        this.particles = [];
        
        this.keys = {};
        this.mouse = { x: 0, y: 0, clicked: false };
        
        // Wave system variables
        this.waveState = 'spawning'; // 'spawning', 'active', 'complete', 'intermission'
        this.waveTimer = 0;
        this.intermissionTimer = 0;
        this.intermissionDuration = 180; // 3 seconds at 60fps
        this.enemiesSpawned = 0;
        this.baseEnemyCount = 8; // Start with 8 enemies in wave 1
        this.waveEnemyCount = this.baseEnemyCount;
        this.enemySpawnDelay = 30; // Frames between enemy spawns
        this.lastSpawnTime = 0;
        
        this.setupEventListeners();
        this.gameLoop();
    }
    
    setupEventListeners() {
        // Keyboard events
        document.addEventListener('keydown', (e) => {
            this.keys[e.code] = true;
        });
        
        document.addEventListener('keyup', (e) => {
            this.keys[e.code] = false;
        });
        
        // Mouse events
        this.canvas.addEventListener('mousemove', (e) => {
            const rect = this.canvas.getBoundingClientRect();
            this.mouse.x = e.clientX - rect.left;
            this.mouse.y = e.clientY - rect.top;
        });
        
        this.canvas.addEventListener('mousedown', (e) => {
            if (this.gameState === 'playing') {
                this.mouse.clicked = true;
                this.player.attack(this.mouse.x, this.mouse.y, this.projectiles);
                this.createLightBurst(this.mouse.x, this.mouse.y);
            }
        });
        
        // UI Button events
        document.getElementById('startBtn').addEventListener('click', () => {
            this.startGame();
        });
        
        document.getElementById('pauseBtn').addEventListener('click', () => {
            this.togglePause();
        });
        
        document.getElementById('restartBtn').addEventListener('click', () => {
            this.restartGame();
        });
    }
    
    startGame() {
        this.gameState = 'playing';
        document.getElementById('startBtn').style.display = 'none';
        document.getElementById('pauseBtn').style.display = 'inline-block';
    }
    
    togglePause() {
        if (this.gameState === 'playing') {
            this.gameState = 'paused';
            document.getElementById('pauseBtn').textContent = 'Resume';
        } else if (this.gameState === 'paused') {
            this.gameState = 'playing';
            document.getElementById('pauseBtn').textContent = 'Pause';
        }
    }
    
    restartGame() {
        this.gameState = 'playing';
        this.score = 0;
        this.wave = 1;
        this.player = new Player(this.width / 2, this.height / 2);
        this.health = this.player.health; // Sync with player health
        this.enemies = [];
        this.projectiles = [];
        this.particles = [];
        
        // Reset wave system
        this.waveState = 'spawning';
        this.waveTimer = 0;
        this.intermissionTimer = 0;
        this.enemiesSpawned = 0;
        this.waveEnemyCount = this.baseEnemyCount;
        this.enemySpawnDelay = 30;
        this.lastSpawnTime = 0;
        
        document.getElementById('game-over').style.display = 'none';
        document.getElementById('pauseBtn').style.display = 'inline-block';
        document.getElementById('pauseBtn').textContent = 'Pause';
        
        this.updateUI();
    }
    
    createLightBurst(x, y) {
        // Visual effect for light magic attack
        for (let i = 0; i < 8; i++) {
            this.particles.push(new Particle(x, y, 'light'));
        }
    }
    
    createSparkleEffect(x, y) {
        // Create sparkle particles when enemy is defeated
        for (let i = 0; i < 12; i++) {
            this.particles.push(new Particle(
                x + (Math.random() - 0.5) * 30,
                y + (Math.random() - 0.5) * 30,
                'sparkle'
            ));
        }
    }
    
    spawnEnemyFromEdge() {
        // Spawn a single enemy from a random edge of the screen
        let x, y;
        const side = Math.floor(Math.random() * 4);
        const margin = 50; // Distance outside screen
        
        switch(side) {
            case 0: // top
                x = Math.random() * this.width;
                y = -margin;
                break;
            case 1: // right
                x = this.width + margin;
                y = Math.random() * this.height;
                break;
            case 2: // bottom
                x = Math.random() * this.width;
                y = this.height + margin;
                break;
            case 3: // left
                x = -margin;
                y = Math.random() * this.height;
                break;
        }
        
        this.enemies.push(new ShadowDemon(x, y, this.wave));
        this.enemiesSpawned++;
    }
    
    updateWaveSystem() {
        this.waveTimer++;
        
        switch(this.waveState) {
            case 'spawning':
                // Spawn enemies for current wave
                if (this.enemiesSpawned < this.waveEnemyCount) {
                    if (this.waveTimer - this.lastSpawnTime >= this.enemySpawnDelay) {
                        this.spawnEnemyFromEdge();
                        this.lastSpawnTime = this.waveTimer;
                    }
                } else {
                    // All enemies spawned, wave is now active
                    this.waveState = 'active';
                }
                break;
                
            case 'active':
                // Check if all enemies are defeated
                if (this.enemies.length === 0) {
                    this.waveState = 'complete';
                    this.showWaveComplete();
                    this.restorePlayerHealth();
                    this.intermissionTimer = 0;
                }
                break;
                
            case 'complete':
                // Brief moment to show completion message
                if (this.waveTimer > this.lastSpawnTime + 60) { // 1 second
                    this.waveState = 'intermission';
                    this.intermissionTimer = 0;
                }
                break;
                
            case 'intermission':
                this.intermissionTimer++;
                if (this.intermissionTimer >= this.intermissionDuration) {
                    this.startNextWave();
                }
                break;
        }
    }
    
    startNextWave() {
        this.wave++;
        this.waveState = 'spawning';
        this.waveTimer = 0;
        this.lastSpawnTime = 0;
        this.enemiesSpawned = 0;
        
        // Increase difficulty
        this.waveEnemyCount = this.baseEnemyCount + (this.wave - 1) * 2; // +2 enemies per wave
        this.enemySpawnDelay = Math.max(15, this.enemySpawnDelay - 2); // Spawn faster (min 15 frames)
        
        this.showWaveTransition();
    }
    
    restorePlayerHealth() {
        // Restore player health between waves
        this.player.health = this.player.maxHealth;
        this.player.lightEnergy = this.player.maxLightEnergy;
        
        // Create healing particles
        for (let i = 0; i < 12; i++) {
            this.particles.push(new Particle(
                this.player.x + (Math.random() - 0.5) * 40,
                this.player.y + (Math.random() - 0.5) * 40,
                'healing'
            ));
        }
    }
    
    showWaveTransition() {
        // Create wave transition effect
        const transition = document.createElement('div');
        transition.className = 'wave-transition';
        transition.textContent = `Wave ${this.wave}!`;
        document.body.appendChild(transition);
        
        setTimeout(() => {
            if (document.body.contains(transition)) {
                document.body.removeChild(transition);
            }
        }, 2000);
    }
    
    showWaveComplete() {
        // Create wave complete effect
        const complete = document.createElement('div');
        complete.className = 'wave-transition';
        complete.textContent = 'Wave Complete!';
        complete.style.background = 'linear-gradient(45deg, #4CAF50, #2E7D32)';
        document.body.appendChild(complete);
        
        setTimeout(() => {
            if (document.body.contains(complete)) {
                document.body.removeChild(complete);
            }
        }, 2000);
    }
    
    checkCollisions() {
        // Projectile vs Enemy collisions
        for (let i = this.projectiles.length - 1; i >= 0; i--) {
            for (let j = this.enemies.length - 1; j >= 0; j--) {
                if (this.projectiles[i] && this.enemies[j] && 
                    this.checkCircleCollision(this.projectiles[i], this.enemies[j])) {
                    
                    // Create sparkle effect when enemy is defeated
                    this.createSparkleEffect(this.enemies[j].x, this.enemies[j].y);
                    
                    // Create explosion particles
                    for (let k = 0; k < 6; k++) {
                        this.particles.push(new Particle(
                            this.enemies[j].x, 
                            this.enemies[j].y, 
                            'explosion'
                        ));
                    }
                    
                    // Remove enemy and projectile
                    this.enemies.splice(j, 1);
                    this.projectiles.splice(i, 1);
                    this.score += 10 * this.wave;
                    break;
                }
            }
        }
        
        // Enemy vs Player collisions - using improved collision detection
        for (let i = this.enemies.length - 1; i >= 0; i--) {
            if (this.player.checkCollision(this.enemies[i])) {
                // Use player's takeDamage method
                this.player.takeDamage(20);
                this.health = this.player.health; // Sync game health with player health
                
                this.enemies.splice(i, 1);
                
                // Create damage particles
                for (let k = 0; k < 6; k++) {
                    this.particles.push(new Particle(
                        this.player.x, 
                        this.player.y, 
                        'damage'
                    ));
                }
                
                if (this.player.health <= 0) {
                    this.gameOver();
                }
                break;
            }
        }
    }
    
    checkCircleCollision(obj1, obj2) {
        const dx = obj1.x - obj2.x;
        const dy = obj1.y - obj2.y;
        const distance = Math.sqrt(dx * dx + dy * dy);
        return distance < obj1.radius + obj2.radius;
    }
    
    gameOver() {
        this.gameState = 'gameOver';
        document.getElementById('final-score').textContent = this.score;
        document.getElementById('game-over').style.display = 'block';
        document.getElementById('pauseBtn').style.display = 'none';
    }
    
    updateUI() {
        // Update health from player object
        this.health = this.player.health;
        document.getElementById('health').textContent = Math.max(0, this.player.health);
        document.getElementById('wave').textContent = this.wave;
        document.getElementById('score').textContent = this.score;
        
        // Add health warning effect
        const healthElement = document.getElementById('health').parentElement;
        if (this.player.health < 30) {
            healthElement.classList.add('health-low');
        } else {
            healthElement.classList.remove('health-low');
        }
        
        // Update light energy display if element exists
        const energyElement = document.getElementById('light-energy');
        if (energyElement) {
            energyElement.textContent = Math.floor(this.player.lightEnergy);
        }
    }
    
    update() {
        if (this.gameState !== 'playing') return;
        
        // Update player
        this.player.update(this.keys, this.width, this.height);
        
        // Update enemies
        this.enemies.forEach(enemy => {
            enemy.update(this.player.x, this.player.y);
        });
        
        // Update projectiles
        for (let i = this.projectiles.length - 1; i >= 0; i--) {
            this.projectiles[i].update();
            if (this.projectiles[i].isOffScreen(this.width, this.height)) {
                this.projectiles.splice(i, 1);
            }
        }
        
        // Update particles
        for (let i = this.particles.length - 1; i >= 0; i--) {
            this.particles[i].update();
            if (this.particles[i].life <= 0) {
                this.particles.splice(i, 1);
            }
        }
        
        // Update wave system
        this.updateWaveSystem();
        
        // Check collisions
        this.checkCollisions();
        
        // Update UI
        this.updateUI();
    }
    
    render() {
        // Clear canvas with sacred ground background
        const gradient = this.ctx.createLinearGradient(0, 0, this.width, this.height);
        gradient.addColorStop(0, '#87CEEB');
        gradient.addColorStop(1, '#98FB98');
        this.ctx.fillStyle = gradient;
        this.ctx.fillRect(0, 0, this.width, this.height);
        
        // Draw sacred circle in center
        this.ctx.strokeStyle = '#FFD700';
        this.ctx.lineWidth = 3;
        this.ctx.beginPath();
        this.ctx.arc(this.width / 2, this.height / 2, 100, 0, Math.PI * 2);
        this.ctx.stroke();
        
        if (this.gameState === 'playing' || this.gameState === 'paused') {
            // Render particles first (background)
            this.particles.forEach(particle => particle.render(this.ctx));
            
            // Render game objects
            this.player.render(this.ctx);
            this.enemies.forEach(enemy => enemy.render(this.ctx));
            this.projectiles.forEach(projectile => projectile.render(this.ctx));
        }
        
        // Draw pause overlay
        if (this.gameState === 'paused') {
            this.ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
            this.ctx.fillRect(0, 0, this.width, this.height);
            this.ctx.fillStyle = '#FFD700';
            this.ctx.font = '48px bold serif';
            this.ctx.textAlign = 'center';
            this.ctx.fillText('PAUSED', this.width / 2, this.height / 2);
        }
        
        // Draw intermission countdown
        if (this.waveState === 'intermission') {
            const remainingTime = Math.ceil((this.intermissionDuration - this.intermissionTimer) / 60);
            this.ctx.fillStyle = 'rgba(0, 0, 0, 0.4)';
            this.ctx.fillRect(0, 0, this.width, this.height);
            this.ctx.fillStyle = '#FFD700';
            this.ctx.font = '24px bold serif';
            this.ctx.textAlign = 'center';
            this.ctx.fillText(`Next Wave in ${remainingTime}...`, this.width / 2, this.height / 2);
        }
        
        // Draw start screen
        if (this.gameState === 'menu') {
            this.ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            this.ctx.fillRect(0, 0, this.width, this.height);
            this.ctx.fillStyle = '#FFD700';
            this.ctx.font = '36px bold serif';
            this.ctx.textAlign = 'center';
            this.ctx.fillText('Defend the Sacred Ground!', this.width / 2, this.height / 2 - 50);
            this.ctx.font = '18px serif';
            this.ctx.fillText('Click Start Game to begin', this.width / 2, this.height / 2 + 20);
        }
    }
    
    gameLoop() {
        this.update();
        this.render();
        requestAnimationFrame(() => this.gameLoop());
    }
}

class Player {
    constructor(x, y) {
        this.x = x;
        this.y = y;
        this.radius = 15;
        this.maxSpeed = 4;
        this.acceleration = 0.3;
        this.friction = 0.85;
        this.color = '#FFD700';
        this.attackCooldown = 0;
        this.health = 100;
        this.maxHealth = 100;
        this.lightEnergy = 100;
        this.maxLightEnergy = 100;
        
        // Velocity components for smooth movement
        this.vx = 0;
        this.vy = 0;
        
        // Collision box (circular)
        this.collisionRadius = this.radius;
        
        // Visual effects
        this.lightPulse = 0;
        this.energyRegenRate = 0.5;
    }
    
    update(keys, canvasWidth, canvasHeight) {
        // Handle input and apply acceleration
        let inputX = 0;
        let inputY = 0;
        
        if (keys['KeyW'] || keys['ArrowUp']) inputY = -1;
        if (keys['KeyS'] || keys['ArrowDown']) inputY = 1;
        if (keys['KeyA'] || keys['ArrowLeft']) inputX = -1;
        if (keys['KeyD'] || keys['ArrowRight']) inputX = 1;
        
        // Normalize diagonal movement
        if (inputX !== 0 && inputY !== 0) {
            inputX *= 0.707; // 1/sqrt(2)
            inputY *= 0.707;
        }
        
        // Apply acceleration based on input
        this.vx += inputX * this.acceleration;
        this.vy += inputY * this.acceleration;
        
        // Apply friction when no input
        if (inputX === 0) this.vx *= this.friction;
        if (inputY === 0) this.vy *= this.friction;
        
        // Limit maximum speed
        const speed = Math.sqrt(this.vx * this.vx + this.vy * this.vy);
        if (speed > this.maxSpeed) {
            this.vx = (this.vx / speed) * this.maxSpeed;
            this.vy = (this.vy / speed) * this.maxSpeed;
        }
        
        // Update position
        this.x += this.vx;
        this.y += this.vy;
        
        // Keep player within bounds with proper collision
        if (this.x - this.collisionRadius < 0) {
            this.x = this.collisionRadius;
            this.vx = 0;
        }
        if (this.x + this.collisionRadius > canvasWidth) {
            this.x = canvasWidth - this.collisionRadius;
            this.vx = 0;
        }
        if (this.y - this.collisionRadius < 0) {
            this.y = this.collisionRadius;
            this.vy = 0;
        }
        if (this.y + this.collisionRadius > canvasHeight) {
            this.y = canvasHeight - this.collisionRadius;
            this.vy = 0;
        }
        
        // Update attack cooldown
        if (this.attackCooldown > 0) this.attackCooldown--;
        
        // Regenerate light energy over time
        if (this.lightEnergy < this.maxLightEnergy) {
            this.lightEnergy = Math.min(this.maxLightEnergy, this.lightEnergy + this.energyRegenRate);
        }
        
        // Update visual effects
        this.lightPulse += 0.15;
    }
    
    // Collision detection method
    getCollisionBox() {
        return {
            x: this.x,
            y: this.y,
            radius: this.collisionRadius
        };
    }
    
    // Check if player collides with another circular object
    checkCollision(other) {
        const dx = this.x - other.x;
        const dy = this.y - other.y;
        const distance = Math.sqrt(dx * dx + dy * dy);
        return distance < (this.collisionRadius + other.radius);
    }
    
    // Take damage method
    takeDamage(amount) {
        this.health = Math.max(0, this.health - amount);
        
        // Visual feedback for taking damage
        this.lightPulse = 0; // Reset pulse for damage flash
    }
    
    // Heal method
    heal(amount) {
        this.health = Math.min(this.maxHealth, this.health + amount);
    }
    
    // Use light energy for attacks
    useLightEnergy(amount) {
        if (this.lightEnergy >= amount) {
            this.lightEnergy -= amount;
            return true;
        }
        return false;
    }
    
    attack(targetX, targetY, projectiles) {
        if (this.attackCooldown > 0) return;
        
        // Check if player has enough light energy
        const energyCost = 10;
        if (!this.useLightEnergy(energyCost)) return;
        
        const angle = Math.atan2(targetY - this.y, targetX - this.x);
        
        // Create improved light orb projectile
        projectiles.push(new LightOrb(this.x, this.y, angle));
        this.attackCooldown = 12; // Slightly faster attack rate
    }
    
    render(ctx) {
        // Dynamic light pulse effect
        const pulseIntensity = 0.3 + Math.sin(this.lightPulse) * 0.2;
        const auraRadius = this.radius * (2 + pulseIntensity);
        
        // Draw pulsing light aura
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, auraRadius);
        gradient.addColorStop(0, `rgba(255, 215, 0, ${0.6 + pulseIntensity})`);
        gradient.addColorStop(0.7, `rgba(255, 215, 0, ${0.3 + pulseIntensity * 0.5})`);
        gradient.addColorStop(1, 'rgba(255, 215, 0, 0)');
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, auraRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Health-based color modification
        const healthRatio = this.health / this.maxHealth;
        let playerColor = this.color;
        let strokeColor = '#FFA500';
        
        if (healthRatio < 0.3) {
            // Flash red when low health
            const flashIntensity = Math.sin(this.lightPulse * 2) * 0.5 + 0.5;
            playerColor = `rgb(${255}, ${215 - flashIntensity * 100}, ${0})`;
            strokeColor = '#FF4500';
        }
        
        // Draw player main body
        ctx.fillStyle = playerColor;
        ctx.strokeStyle = strokeColor;
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw inner light core
        const coreGradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, this.radius * 0.6);
        coreGradient.addColorStop(0, '#FFFFFF');
        coreGradient.addColorStop(0.5, '#FFFF00');
        coreGradient.addColorStop(1, 'rgba(255, 255, 0, 0.8)');
        ctx.fillStyle = coreGradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius * 0.6, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw health indicator (small bar above player)
        if (this.health < this.maxHealth) {
            const barWidth = this.radius * 2;
            const barHeight = 4;
            const barX = this.x - barWidth / 2;
            const barY = this.y - this.radius - 10;
            
            // Background bar
            ctx.fillStyle = 'rgba(0, 0, 0, 0.5)';
            ctx.fillRect(barX - 1, barY - 1, barWidth + 2, barHeight + 2);
            
            // Health bar
            const healthWidth = (this.health / this.maxHealth) * barWidth;
            ctx.fillStyle = healthRatio > 0.6 ? '#4CAF50' : healthRatio > 0.3 ? '#FF9800' : '#F44336';
            ctx.fillRect(barX, barY, healthWidth, barHeight);
        }
        
        // Draw light energy indicator (small bar below player)
        const energyRatio = this.lightEnergy / this.maxLightEnergy;
        if (energyRatio < 1.0) {
            const barWidth = this.radius * 2;
            const barHeight = 3;
            const barX = this.x - barWidth / 2;
            const barY = this.y + this.radius + 8;
            
            // Background bar
            ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            ctx.fillRect(barX - 1, barY - 1, barWidth + 2, barHeight + 2);
            
            // Energy bar
            const energyWidth = energyRatio * barWidth;
            ctx.fillStyle = '#2196F3';
            ctx.fillRect(barX, barY, energyWidth, barHeight);
        }
    }
}

class ShadowDemon {
    constructor(x, y, wave) {
        this.x = x;
        this.y = y;
        this.radius = 12 + Math.min(wave - 1, 8); // Grow with waves (max +8)
        this.speed = 0.7 + (wave - 1) * 0.08; // Increase speed with waves
        this.color = '#4A0E4E';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 1; // Basic shadow demons have 1 health
    }
    
    update(playerX, playerY) {
        // Move towards player with slight variation
        const angle = Math.atan2(playerY - this.y, playerX - this.x);
        const wobble = Math.sin(this.shadowPulse) * 0.1;
        this.x += Math.cos(angle + wobble) * this.speed;
        this.y += Math.sin(angle + wobble) * this.speed;
        
        // Update shadow pulse for visual effect
        this.shadowPulse += 0.1;
    }
    
    render(ctx) {
        // Dynamic shadow pulse effect
        const pulseIntensity = 0.4 + Math.sin(this.shadowPulse) * 0.3;
        const shadowRadius = this.radius * (2 + pulseIntensity * 0.5);
        
        // Draw pulsing shadow aura
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, shadowRadius);
        gradient.addColorStop(0, `rgba(74, 14, 78, ${0.8 + pulseIntensity * 0.2})`);
        gradient.addColorStop(0.6, `rgba(74, 14, 78, ${0.4 + pulseIntensity * 0.1})`);
        gradient.addColorStop(1, 'rgba(74, 14, 78, 0)');
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, shadowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw main body with wave-based color intensity
        const waveIntensity = Math.min(this.wave / 5, 1);
        const bodyColor = `rgb(${74 + waveIntensity * 50}, ${14 + waveIntensity * 20}, ${78 + waveIntensity * 50})`;
        ctx.fillStyle = bodyColor;
        ctx.strokeStyle = '#2C0A2E';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw menacing eyes
        const eyeSize = Math.max(2, this.radius / 6);
        ctx.fillStyle = '#FF0000';
        ctx.beginPath();
        ctx.arc(this.x - this.radius * 0.3, this.y - this.radius * 0.2, eyeSize, 0, Math.PI * 2);
        ctx.arc(this.x + this.radius * 0.3, this.y - this.radius * 0.2, eyeSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw eye glow effect
        ctx.fillStyle = 'rgba(255, 0, 0, 0.5)';
        ctx.beginPath();
        ctx.arc(this.x - this.radius * 0.3, this.y - this.radius * 0.2, eyeSize * 1.5, 0, Math.PI * 2);
        ctx.arc(this.x + this.radius * 0.3, this.y - this.radius * 0.2, eyeSize * 1.5, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw shadow tendrils for higher waves
        if (this.wave > 2) {
            ctx.strokeStyle = `rgba(74, 14, 78, ${0.6 + pulseIntensity * 0.2})`;
            ctx.lineWidth = 2;
            ctx.lineCap = 'round';
            
            for (let i = 0; i < 4; i++) {
                const tendrilAngle = (Math.PI * 2 / 4) * i + this.shadowPulse * 0.5;
                const tendrilLength = this.radius * 1.5;
                const startX = this.x + Math.cos(tendrilAngle) * this.radius;
                const startY = this.y + Math.sin(tendrilAngle) * this.radius;
                const endX = startX + Math.cos(tendrilAngle) * tendrilLength;
                const endY = startY + Math.sin(tendrilAngle) * tendrilLength;
                
                ctx.beginPath();
                ctx.moveTo(startX, startY);
                ctx.lineTo(endX, endY);
                ctx.stroke();
            }
        }
    }
}

// Keep old Enemy class for compatibility
class Enemy extends ShadowDemon {
    constructor(x, y, wave) {
        super(x, y, wave);
    }
}

class LightOrb {
    constructor(x, y, angle) {
        this.x = x;
        this.y = y;
        this.radius = 6;
        this.speed = 10;
        this.angle = angle;
        this.vx = Math.cos(angle) * this.speed;
        this.vy = Math.sin(angle) * this.speed;
        this.color = '#FFFF00';
        this.life = 120; // Frames before auto-destruct
        this.maxLife = 120;
        this.pulseTimer = 0;
        this.trailParticles = [];
    }
    
    update() {
        // Update position
        this.x += this.vx;
        this.y += this.vy;
        this.life--;
        this.pulseTimer += 0.3;
        
        // Add trail particle
        if (this.trailParticles.length < 8) {
            this.trailParticles.push({
                x: this.x + (Math.random() - 0.5) * 4,
                y: this.y + (Math.random() - 0.5) * 4,
                life: 15,
                maxLife: 15
            });
        }
        
        // Update trail particles
        for (let i = this.trailParticles.length - 1; i >= 0; i--) {
            this.trailParticles[i].life--;
            if (this.trailParticles[i].life <= 0) {
                this.trailParticles.splice(i, 1);
            }
        }
    }
    
    isOffScreen(width, height) {
        return this.x < -50 || this.x > width + 50 || this.y < -50 || this.y > height + 50 || this.life <= 0;
    }
    
    render(ctx) {
        // Draw trail particles
        this.trailParticles.forEach(particle => {
            const alpha = particle.life / particle.maxLife;
            const size = alpha * 3;
            
            ctx.save();
            ctx.globalAlpha = alpha * 0.6;
            ctx.fillStyle = '#FFD700';
            ctx.beginPath();
            ctx.arc(particle.x, particle.y, size, 0, Math.PI * 2);
            ctx.fill();
            ctx.restore();
        });
        
        // Dynamic pulse effect
        const pulseIntensity = 0.5 + Math.sin(this.pulseTimer) * 0.3;
        const glowRadius = this.radius * (2 + pulseIntensity);
        
        // Draw outer glow
        const outerGradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, glowRadius);
        outerGradient.addColorStop(0, 'rgba(255, 255, 255, 0.8)');
        outerGradient.addColorStop(0.3, 'rgba(255, 255, 0, 0.6)');
        outerGradient.addColorStop(0.7, 'rgba(255, 215, 0, 0.3)');
        outerGradient.addColorStop(1, 'rgba(255, 215, 0, 0)');
        ctx.fillStyle = outerGradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, glowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw main orb
        const orbGradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, this.radius);
        orbGradient.addColorStop(0, '#FFFFFF');
        orbGradient.addColorStop(0.4, '#FFFF00');
        orbGradient.addColorStop(0.8, '#FFD700');
        orbGradient.addColorStop(1, '#FFA500');
        ctx.fillStyle = orbGradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw bright core
        ctx.fillStyle = 'rgba(255, 255, 255, 0.9)';
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius * 0.4, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw outer ring
        ctx.strokeStyle = '#FFD700';
        ctx.lineWidth = 2;
        ctx.globalAlpha = pulseIntensity;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius + 2, 0, Math.PI * 2);
        ctx.stroke();
        ctx.globalAlpha = 1;
    }
}

// Keep old Projectile class for compatibility
class Projectile extends LightOrb {
    constructor(x, y, angle) {
        super(x, y, angle);
    }
}

class Particle {
    constructor(x, y, type) {
        this.x = x;
        this.y = y;
        this.type = type;
        this.life = 30;
        this.maxLife = 30;
        
        switch(type) {
            case 'light':
                this.vx = (Math.random() - 0.5) * 4;
                this.vy = (Math.random() - 0.5) * 4;
                this.color = '#FFD700';
                break;
            case 'explosion':
                this.vx = (Math.random() - 0.5) * 6;
                this.vy = (Math.random() - 0.5) * 6;
                this.color = '#FF6B35';
                break;
            case 'damage':
                this.vx = (Math.random() - 0.5) * 3;
                this.vy = (Math.random() - 0.5) * 3;
                this.color = '#E74C3C';
                break;
            case 'sparkle':
                this.vx = (Math.random() - 0.5) * 5;
                this.vy = (Math.random() - 0.5) * 5 - 2; // Slight upward bias
                this.color = '#FFD700';
                this.life = 40; // Longer life for sparkles
                this.maxLife = 40;
                break;
            case 'healing':
                this.vx = (Math.random() - 0.5) * 2;
                this.vy = (Math.random() - 0.5) * 2 - 1; // Float upward
                this.color = '#4CAF50';
                this.life = 45; // Longer life for healing effect
                this.maxLife = 45;
                break;
        }
    }
    
    update() {
        this.x += this.vx;
        this.y += this.vy;
        this.vx *= 0.98;
        this.vy *= 0.98;
        this.life--;
    }
    
    render(ctx) {
        const alpha = this.life / this.maxLife;
        let size = (this.life / this.maxLife) * 6;
        
        ctx.save();
        ctx.globalAlpha = alpha;
        
        switch(this.type) {
            case 'sparkle':
                // Draw sparkle as a star
                size = alpha * 8;
                ctx.fillStyle = this.color;
                ctx.strokeStyle = '#FFFFFF';
                ctx.lineWidth = 1;
                
                // Draw 4-pointed star
                ctx.beginPath();
                for (let i = 0; i < 8; i++) {
                    const angle = (i * Math.PI) / 4;
                    const radius = i % 2 === 0 ? size : size * 0.4;
                    const x = this.x + Math.cos(angle) * radius;
                    const y = this.y + Math.sin(angle) * radius;
                    
                    if (i === 0) ctx.moveTo(x, y);
                    else ctx.lineTo(x, y);
                }
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
                break;
                
            case 'healing':
                // Draw healing as a glowing cross
                size = alpha * 5;
                ctx.fillStyle = this.color;
                ctx.shadowColor = this.color;
                ctx.shadowBlur = 10;
                
                // Draw cross
                ctx.fillRect(this.x - size, this.y - size/3, size*2, size*2/3);
                ctx.fillRect(this.x - size/3, this.y - size, size*2/3, size*2);
                break;
                
            default:
                // Default circular particle
                ctx.fillStyle = this.color;
                ctx.beginPath();
                ctx.arc(this.x, this.y, size, 0, Math.PI * 2);
                ctx.fill();
                break;
        }
        
        ctx.restore();
    }
}

// Initialize the game when the page loads
window.addEventListener('load', () => {
    new Game();
});