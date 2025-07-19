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
        this.weaponShrines = [];
        this.activeWeapon = null;
        this.soundManager = new SoundManager();
        
        this.keys = {};
        this.mouse = { x: 0, y: 0, clicked: false };
        
        // Mobile controls
        this.isMobile = this.detectMobile();
        this.virtualJoystick = {
            active: false,
            centerX: 0,
            centerY: 0,
            knobX: 0,
            knobY: 0,
            inputX: 0,
            inputY: 0
        };
        this.touchButtons = {
            fire: false,
            weapon: false
        };
        
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
        
        this.setupWeaponShrines();
        this.setupCanvas();
        this.setupEventListeners();
        if (this.isMobile) {
            this.setupMobileControls();
        }
        this.gameLoop();
    }
    
    detectMobile() {
        return /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ||
               'ontouchstart' in window ||
               navigator.maxTouchPoints > 0;
    }
    
    setupCanvas() {
        // Make canvas responsive
        this.resizeCanvas();
        window.addEventListener('resize', () => this.resizeCanvas());
        window.addEventListener('orientationchange', () => {
            setTimeout(() => this.resizeCanvas(), 100);
        });
    }
    
    resizeCanvas() {
        const container = document.getElementById('game-container');
        const canvas = this.canvas;
        
        if (this.isMobile) {
            // Mobile responsive sizing
            const maxWidth = window.innerWidth - 20;
            const maxHeight = window.innerHeight - 120;
            
            // Maintain aspect ratio
            let canvasWidth = 800;
            let canvasHeight = 600;
            const aspectRatio = canvasWidth / canvasHeight;
            
            if (maxWidth / aspectRatio <= maxHeight) {
                canvasWidth = maxWidth;
                canvasHeight = maxWidth / aspectRatio;
            } else {
                canvasHeight = maxHeight;
                canvasWidth = maxHeight * aspectRatio;
            }
            
            canvas.style.width = canvasWidth + 'px';
            canvas.style.height = canvasHeight + 'px';
        } else {
            // Desktop sizing
            canvas.style.width = '800px';
            canvas.style.height = '600px';
        }
    }
    
    setupWeaponShrines() {
        // Create 4 weapon shrines at corners of the map
        const margin = 60; // Distance from edges
        const weaponTypes = ['thunder_hammer', 'phoenix_bow', 'shield_of_light', 'spirit_cannon'];
        
        // Top-left
        this.weaponShrines.push(new WeaponShrine(margin, margin, weaponTypes[0]));
        // Top-right  
        this.weaponShrines.push(new WeaponShrine(this.width - margin, margin, weaponTypes[1]));
        // Bottom-left
        this.weaponShrines.push(new WeaponShrine(margin, this.height - margin, weaponTypes[2]));
        // Bottom-right
        this.weaponShrines.push(new WeaponShrine(this.width - margin, this.height - margin, weaponTypes[3]));
    }
    
    setupMobileControls() {
        // Update instructions for mobile
        const controlText = document.getElementById('control-text');
        if (controlText) {
            controlText.textContent = 'Touch to Start - Use joystick to move';
        }
        
        const startBtn = document.getElementById('startBtn');
        if (startBtn) {
            startBtn.textContent = 'Touch to Start';
        }
        
        // Setup virtual joystick
        const joystick = document.getElementById('virtual-joystick');
        const knob = document.getElementById('joystick-knob');
        
        if (joystick) {
            const rect = joystick.getBoundingClientRect();
            this.virtualJoystick.centerX = rect.left + rect.width / 2;
            this.virtualJoystick.centerY = rect.top + rect.height / 2;
            
            // Joystick touch events
            joystick.addEventListener('touchstart', (e) => {
                e.preventDefault();
                this.virtualJoystick.active = true;
                this.handleJoystickTouch(e.touches[0]);
            });
            
            joystick.addEventListener('touchmove', (e) => {
                e.preventDefault();
                if (this.virtualJoystick.active) {
                    this.handleJoystickTouch(e.touches[0]);
                }
            });
            
            joystick.addEventListener('touchend', (e) => {
                e.preventDefault();
                this.virtualJoystick.active = false;
                this.virtualJoystick.inputX = 0;
                this.virtualJoystick.inputY = 0;
                this.resetJoystickKnob();
            });
        }
        
        // Setup mobile buttons
        const fireBtn = document.getElementById('fire-btn');
        const weaponBtn = document.getElementById('weapon-btn');
        
        if (fireBtn) {
            fireBtn.addEventListener('touchstart', (e) => {
                e.preventDefault();
                this.touchButtons.fire = true;
                fireBtn.classList.add('pressed');
                this.handleMobileAttack();
            });
            
            fireBtn.addEventListener('touchend', (e) => {
                e.preventDefault();
                this.touchButtons.fire = false;
                fireBtn.classList.remove('pressed');
            });
        }
        
        if (weaponBtn) {
            weaponBtn.addEventListener('touchstart', (e) => {
                e.preventDefault();
                this.touchButtons.weapon = true;
                weaponBtn.classList.add('pressed');
                this.handleWeaponAbility();
            });
            
            weaponBtn.addEventListener('touchend', (e) => {
                e.preventDefault();
                this.touchButtons.weapon = false;
                weaponBtn.classList.remove('pressed');
            });
        }
        
        // Prevent scrolling on mobile
        document.addEventListener('touchmove', (e) => {
            e.preventDefault();
        }, { passive: false });
    }
    
    handleJoystickTouch(touch) {
        const joystick = document.getElementById('virtual-joystick');
        const knob = document.getElementById('joystick-knob');
        const rect = joystick.getBoundingClientRect();
        
        const centerX = rect.left + rect.width / 2;
        const centerY = rect.top + rect.height / 2;
        const maxDistance = rect.width / 2 - 20;
        
        let deltaX = touch.clientX - centerX;
        let deltaY = touch.clientY - centerY;
        const distance = Math.sqrt(deltaX * deltaX + deltaY * deltaY);
        
        if (distance > maxDistance) {
            deltaX = (deltaX / distance) * maxDistance;
            deltaY = (deltaY / distance) * maxDistance;
        }
        
        this.virtualJoystick.knobX = deltaX;
        this.virtualJoystick.knobY = deltaY;
        this.virtualJoystick.inputX = deltaX / maxDistance;
        this.virtualJoystick.inputY = deltaY / maxDistance;
        
        // Update knob position
        if (knob) {
            knob.style.transform = `translate(calc(-50% + ${deltaX}px), calc(-50% + ${deltaY}px))`;
        }
    }
    
    resetJoystickKnob() {
        const knob = document.getElementById('joystick-knob');
        if (knob) {
            knob.style.transform = 'translate(-50%, -50%)';
        }
    }
    
    handleMobileAttack() {
        if (this.gameState === 'playing') {
            // Attack towards center of screen or last touch position
            const centerX = this.width / 2;
            const centerY = this.height / 2;
            
            if (this.activeWeapon) {
                this.activeWeapon.attack(this.player, centerX, centerY, this.projectiles, this.particles);
            } else {
                this.player.attack(centerX, centerY, this.projectiles);
                this.createLightBurst(centerX, centerY);
                this.soundManager.playSound('shoot', true);
            }
        }
    }
    
    handleWeaponAbility() {
        if (this.gameState === 'playing' && this.activeWeapon) {
            // Use weapon special ability if available
            const centerX = this.width / 2;
            const centerY = this.height / 2;
            this.activeWeapon.attack(this.player, centerX, centerY, this.projectiles, this.particles);
        }
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
                
                // Use weapon if player has one, otherwise default light orb
                if (this.activeWeapon) {
                    this.activeWeapon.attack(this.player, this.mouse.x, this.mouse.y, this.projectiles, this.particles);
                } else {
                    this.player.attack(this.mouse.x, this.mouse.y, this.projectiles);
                    this.createLightBurst(this.mouse.x, this.mouse.y);
                    this.soundManager.playSound('shoot', true);
                }
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
        
        document.getElementById('muteBtn').addEventListener('click', () => {
            this.toggleSound();
        });
    }
    
    startGame() {
        this.gameState = 'playing';
        document.getElementById('startBtn').style.display = 'none';
        document.getElementById('pauseBtn').style.display = 'inline-block';
        
        // Google Analytics event tracking
        if (typeof gtag !== 'undefined') {
            gtag('event', 'game_play', { 
                'game_name': 'Phoenix Light Warrior',
                'event_category': 'game_interaction',
                'event_label': 'start_game'
            });
        }
        
        // Start background music when game starts
        this.soundManager.startBackgroundMusic();
    }
    
    toggleSound() {
        const enabled = this.soundManager.toggleMute();
        const muteBtn = document.getElementById('muteBtn');
        muteBtn.textContent = enabled ? '🔊' : '🔇';
        muteBtn.className = enabled ? 'sound-btn' : 'sound-btn muted';
        
        if (enabled && this.gameState === 'playing') {
            this.soundManager.startBackgroundMusic();
        }
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
        
        // Google Analytics event tracking
        if (typeof gtag !== 'undefined') {
            gtag('event', 'game_play', { 
                'game_name': 'Phoenix Light Warrior',
                'event_category': 'game_interaction',
                'event_label': 'restart_game'
            });
        }
        
        // Reset wave system
        this.waveState = 'spawning';
        this.waveTimer = 0;
        this.intermissionTimer = 0;
        this.enemiesSpawned = 0;
        this.waveEnemyCount = this.baseEnemyCount;
        this.enemySpawnDelay = 30;
        this.lastSpawnTime = 0;
        
        // Reset weapon system
        this.activeWeapon = null;
        this.weaponShrines.forEach(shrine => shrine.reset());
        
        // Restart background music
        if (this.soundManager.enabled) {
            this.soundManager.startBackgroundMusic();
        }
        
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
    
    checkWeaponPickups() {
        // Check if player is near any weapon shrine
        for (let shrine of this.weaponShrines) {
            if (shrine.isActive && this.checkCircleCollision(this.player, shrine)) {
                // Pick up weapon
                this.activeWeapon = shrine.pickupWeapon();
                
                // Play weapon pickup sound
                this.soundManager.playSound('weapon_pickup');
                
                // Create pickup particles
                for (let i = 0; i < 15; i++) {
                    this.particles.push(new Particle(
                        shrine.x + (Math.random() - 0.5) * 40,
                        shrine.y + (Math.random() - 0.5) * 40,
                        'weapon_pickup'
                    ));
                }
                break;
            }
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
                    this.soundManager.playSound('wave_complete');
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
        
        // Google Analytics event tracking for wave progression
        if (typeof gtag !== 'undefined') {
            gtag('event', 'level_up', { 
                'game_name': 'Phoenix Light Warrior',
                'event_category': 'game_progression',
                'event_label': 'wave_complete',
                'value': this.wave
            });
        }
        
        // Reset weapon system for new wave
        this.activeWeapon = null;
        this.weaponShrines.forEach(shrine => shrine.reset());
        
        // Increase difficulty - Wave 1: 8, Wave 2: 12, Wave 3: 16, etc.
        this.waveEnemyCount = this.baseEnemyCount + (this.wave - 1) * 4; // +4 enemies per wave
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
                    
                    // Play enemy defeat sound
                    this.soundManager.playSound('enemy_defeat', true);
                    
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
                
                // Play hurt sound
                this.soundManager.playSound('player_hurt');
                
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
        
        // Google Analytics event tracking
        if (typeof gtag !== 'undefined') {
            gtag('event', 'game_over', { 
                'game_name': 'Phoenix Light Warrior',
                'event_category': 'game_interaction',
                'event_label': 'game_over',
                'value': this.score
            });
        }
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
        
        // Update weapon display
        const weaponDisplay = document.getElementById('weapon-display');
        const activeWeaponSpan = document.getElementById('active-weapon');
        if (this.activeWeapon) {
            weaponDisplay.style.display = 'block';
            activeWeaponSpan.textContent = this.getWeaponName(this.activeWeapon.type);
        } else {
            weaponDisplay.style.display = 'none';
        }
    }
    
    getWeaponName(weaponType) {
        switch(weaponType) {
            case 'thunder_hammer': return 'Thunder Hammer';
            case 'phoenix_bow': return 'Phoenix Bow';
            case 'shield_of_light': return 'Shield of Light';
            case 'spirit_cannon': return 'Spirit Cannon';
            default: return 'Unknown Weapon';
        }
    }
    
    update() {
        if (this.gameState !== 'playing') return;
        
        // Update player with mobile input support
        this.player.update(this.keys, this.width, this.height, this.isMobile ? this.virtualJoystick : null);
        
        // Update active weapon
        if (this.activeWeapon) {
            this.activeWeapon.update();
        }
        
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
        this.checkWeaponPickups();
        
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
            // Render weapon shrines first (background layer)
            this.weaponShrines.forEach(shrine => shrine.render(this.ctx));
            
            // Render particles
            this.particles.forEach(particle => particle.render(this.ctx));
            
            // Render game objects
            this.player.render(this.ctx);
            
            // Render shield if active
            if (this.activeWeapon && this.activeWeapon.type === 'shield_of_light') {
                this.activeWeapon.render(this.ctx, this.player);
            }
            
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
        
        // Draw wave status information
        if (this.waveState === 'spawning' && this.gameState === 'playing') {
            // Show spawning progress
            const remaining = this.waveEnemyCount - this.enemiesSpawned;
            this.ctx.fillStyle = 'rgba(0, 0, 0, 0.3)';
            this.ctx.fillRect(10, this.height - 40, 200, 30);
            this.ctx.fillStyle = '#FFD700';
            this.ctx.font = '16px bold serif';
            this.ctx.textAlign = 'left';
            this.ctx.fillText(`Spawning: ${remaining} enemies left`, 20, this.height - 20);
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
            this.ctx.font = '18px serif';
            this.ctx.fillText(`Wave ${this.wave + 1}: ${this.baseEnemyCount + this.wave * 4} Shadow Demons`, this.width / 2, this.height / 2 + 40);
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
    
    update(keys, canvasWidth, canvasHeight, virtualJoystick = null) {
        // Handle input and apply acceleration
        let inputX = 0;
        let inputY = 0;
        
        // Handle mobile virtual joystick input
        if (virtualJoystick && virtualJoystick.active) {
            inputX = virtualJoystick.inputX;
            inputY = virtualJoystick.inputY;
        } else {
            // Handle keyboard input
            if (keys['KeyW'] || keys['ArrowUp']) inputY = -1;
            if (keys['KeyS'] || keys['ArrowDown']) inputY = 1;
            if (keys['KeyA'] || keys['ArrowLeft']) inputX = -1;
            if (keys['KeyD'] || keys['ArrowRight']) inputX = 1;
        }
        
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
            case 'weapon_pickup':
                this.vx = (Math.random() - 0.5) * 6;
                this.vy = (Math.random() - 0.5) * 6 - 3; // Float upward with more velocity
                this.color = '#9C27B0'; // Purple for weapon pickup
                this.life = 50;
                this.maxLife = 50;
                break;
            case 'lightning':
                this.vx = (Math.random() - 0.5) * 8;
                this.vy = (Math.random() - 0.5) * 8;
                this.color = '#FFD700';
                this.life = 20; // Short but intense
                this.maxLife = 20;
                break;
            case 'shield':
                this.vx = (Math.random() - 0.5) * 3;
                this.vy = (Math.random() - 0.5) * 3;
                this.color = '#00BFFF';
                this.life = 60;
                this.maxLife = 60;
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

class WeaponShrine {
    constructor(x, y, weaponType) {
        this.x = x;
        this.y = y;
        this.radius = 25;
        this.weaponType = weaponType;
        this.isActive = true;
        this.pulseTimer = Math.random() * Math.PI * 2;
        this.glowIntensity = 0;
    }
    
    reset() {
        this.isActive = true;
    }
    
    pickupWeapon() {
        this.isActive = false;
        return new this.getWeaponClass()(this.weaponType);
    }
    
    getWeaponClass() {
        switch(this.weaponType) {
            case 'thunder_hammer': return ThunderHammer;
            case 'phoenix_bow': return PhoenixBow;
            case 'shield_of_light': return ShieldOfLight;
            case 'spirit_cannon': return SpiritCannon;
            default: return ThunderHammer;
        }
    }
    
    getWeaponName() {
        switch(this.weaponType) {
            case 'thunder_hammer': return 'Thunder Hammer';
            case 'phoenix_bow': return 'Phoenix Bow';
            case 'shield_of_light': return 'Shield of Light';
            case 'spirit_cannon': return 'Spirit Cannon';
            default: return 'Unknown Weapon';
        }
    }
    
    getWeaponColor() {
        switch(this.weaponType) {
            case 'thunder_hammer': return '#FFD700'; // Gold
            case 'phoenix_bow': return '#FF4500'; // Red-orange
            case 'shield_of_light': return '#00BFFF'; // Sky blue
            case 'spirit_cannon': return '#9932CC'; // Purple
            default: return '#FFD700';
        }
    }
    
    update() {
        this.pulseTimer += 0.1;
        this.glowIntensity = 0.5 + Math.sin(this.pulseTimer) * 0.3;
    }
    
    render(ctx) {
        if (!this.isActive) return;
        
        this.update();
        
        const weaponColor = this.getWeaponColor();
        
        // Draw glowing pedestal base
        const gradient = ctx.createRadialGradient(this.x, this.y + 10, 0, this.x, this.y + 10, this.radius * 2);
        gradient.addColorStop(0, `${weaponColor}80`);
        gradient.addColorStop(1, `${weaponColor}00`);
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y + 10, this.radius * 2, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw pedestal
        ctx.fillStyle = '#444';
        ctx.strokeStyle = '#666';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(this.x, this.y + 10, this.radius * 0.8, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw weapon symbol
        const symbolSize = this.radius * 0.5 + Math.sin(this.pulseTimer) * 3;
        ctx.fillStyle = weaponColor;
        ctx.strokeStyle = '#FFF';
        ctx.lineWidth = 2;
        
        switch(this.weaponType) {
            case 'thunder_hammer':
                // Draw hammer symbol
                ctx.fillRect(this.x - symbolSize/2, this.y - 5, symbolSize, 8);
                ctx.fillRect(this.x - 3, this.y - symbolSize/2, 6, symbolSize);
                break;
                
            case 'phoenix_bow':
                // Draw bow symbol
                ctx.beginPath();
                ctx.arc(this.x, this.y, symbolSize, Math.PI * 0.3, Math.PI * 1.7);
                ctx.stroke();
                ctx.beginPath();
                ctx.moveTo(this.x - symbolSize * 0.7, this.y);
                ctx.lineTo(this.x + symbolSize * 0.7, this.y);
                ctx.stroke();
                break;
                
            case 'shield_of_light':
                // Draw shield symbol
                ctx.beginPath();
                ctx.moveTo(this.x, this.y - symbolSize);
                ctx.lineTo(this.x - symbolSize * 0.7, this.y - symbolSize * 0.3);
                ctx.lineTo(this.x - symbolSize * 0.7, this.y + symbolSize * 0.3);
                ctx.lineTo(this.x, this.y + symbolSize);
                ctx.lineTo(this.x + symbolSize * 0.7, this.y + symbolSize * 0.3);
                ctx.lineTo(this.x + symbolSize * 0.7, this.y - symbolSize * 0.3);
                ctx.closePath();
                ctx.fill();
                ctx.stroke();
                break;
                
            case 'spirit_cannon':
                // Draw cannon symbol
                ctx.fillRect(this.x - symbolSize, this.y - 4, symbolSize * 1.5, 8);
                ctx.beginPath();
                ctx.arc(this.x - symbolSize * 0.5, this.y, symbolSize * 0.4, 0, Math.PI * 2);
                ctx.fill();
                break;
        }
        
        // Draw weapon name
        ctx.fillStyle = weaponColor;
        ctx.font = '12px bold serif';
        ctx.textAlign = 'center';
        ctx.fillText(this.getWeaponName(), this.x, this.y + this.radius + 15);
    }
}

// Base Weapon class
class Weapon {
    constructor(type) {
        this.type = type;
        this.attackCooldown = 0;
    }
    
    update() {
        if (this.attackCooldown > 0) this.attackCooldown--;
    }
    
    canAttack() {
        return this.attackCooldown <= 0;
    }
    
    attack(player, targetX, targetY, projectiles, particles) {
        // Override in subclasses
    }
}

class ThunderHammer extends Weapon {
    constructor() {
        super('thunder_hammer');
        this.cooldownTime = 45; // Slower attack rate for AOE
        this.range = 80;
        this.damage = 50;
    }
    
    attack(player, targetX, targetY, projectiles, particles) {
        if (!this.canAttack()) return;
        
        this.attackCooldown = this.cooldownTime;
        
        // Play thunder hammer sound
        game.soundManager.playSound('thunder_hammer');
        
        // Create AOE damage around player
        const enemies = game.enemies; // Access game enemies
        
        for (let i = enemies.length - 1; i >= 0; i--) {
            const enemy = enemies[i];
            const dx = enemy.x - player.x;
            const dy = enemy.y - player.y;
            const distance = Math.sqrt(dx * dx + dy * dy);
            
            if (distance <= this.range) {
                // Create thunder effect
                for (let j = 0; j < 8; j++) {
                    particles.push(new Particle(enemy.x, enemy.y, 'explosion'));
                }
                game.createSparkleEffect(enemy.x, enemy.y);
                enemies.splice(i, 1);
                game.score += 15 * game.wave; // Bonus points for AOE weapon
            }
        }
        
        // Create thunder visual effect
        for (let i = 0; i < 20; i++) {
            particles.push(new Particle(
                player.x + (Math.random() - 0.5) * this.range * 2,
                player.y + (Math.random() - 0.5) * this.range * 2,
                'lightning'
            ));
        }
    }
}

class PhoenixBow extends Weapon {
    constructor() {
        super('phoenix_bow');
        this.cooldownTime = 20;
        this.arrowCount = 3;
    }
    
    attack(player, targetX, targetY, projectiles, particles) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(15)) return; // Higher energy cost
        
        this.attackCooldown = this.cooldownTime;
        
        // Play phoenix bow sound
        game.soundManager.playSound('phoenix_bow');
        
        const centerAngle = Math.atan2(targetY - player.y, targetX - player.x);
        const spread = Math.PI / 6; // 30 degree spread
        
        // Fire 3 arrows in a spread
        for (let i = 0; i < this.arrowCount; i++) {
            const angle = centerAngle + (i - 1) * spread / 2;
            projectiles.push(new PhoenixArrow(player.x, player.y, angle));
        }
        
        // Create bow effect particles
        for (let i = 0; i < 6; i++) {
            particles.push(new Particle(player.x, player.y, 'light'));
        }
    }
}

class ShieldOfLight extends Weapon {
    constructor() {
        super('shield_of_light');
        this.cooldownTime = 10;
        this.isActive = false;
        this.shieldDuration = 120; // 2 seconds
        this.shieldTimer = 0;
    }
    
    update() {
        super.update();
        if (this.isActive) {
            this.shieldTimer--;
            if (this.shieldTimer <= 0) {
                this.isActive = false;
            }
        }
    }
    
    attack(player, targetX, targetY, projectiles, particles) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(20)) return;
        
        this.attackCooldown = this.cooldownTime;
        this.isActive = true;
        this.shieldTimer = this.shieldDuration;
        
        // Create shield particles
        for (let i = 0; i < 12; i++) {
            particles.push(new Particle(
                player.x + Math.cos(i * Math.PI / 6) * 30,
                player.y + Math.sin(i * Math.PI / 6) * 30,
                'shield'
            ));
        }
    }
    
    render(ctx, player) {
        if (!this.isActive) return;
        
        // Draw protective shield around player
        const alpha = this.shieldTimer / this.shieldDuration;
        const radius = 35 + Math.sin(Date.now() * 0.01) * 5;
        
        ctx.save();
        ctx.globalAlpha = alpha * 0.3;
        ctx.strokeStyle = '#00BFFF';
        ctx.lineWidth = 4;
        ctx.beginPath();
        ctx.arc(player.x, player.y, radius, 0, Math.PI * 2);
        ctx.stroke();
        ctx.restore();
    }
}

class SpiritCannon extends Weapon {
    constructor() {
        super('spirit_cannon');
        this.cooldownTime = 5; // Very fast firing
        this.burstCount = 0;
        this.maxBurst = 8;
        this.burstCooldown = 60; // Cooldown between bursts
    }
    
    attack(player, targetX, targetY, projectiles, particles) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(3)) return; // Low energy per shot
        
        this.attackCooldown = this.cooldownTime;
        this.burstCount++;
        
        // Play spirit cannon sound
        game.soundManager.playSound('spirit_cannon');
        
        const angle = Math.atan2(targetY - player.y, targetX - player.x);
        projectiles.push(new SpiritBlast(player.x, player.y, angle));
        
        if (this.burstCount >= this.maxBurst) {
            this.burstCount = 0;
            this.attackCooldown = this.burstCooldown;
        }
        
        // Create muzzle flash
        particles.push(new Particle(player.x, player.y, 'light'));
    }
}

// Special projectile classes
class PhoenixArrow extends LightOrb {
    constructor(x, y, angle) {
        super(x, y, angle);
        this.speed = 12;
        this.vx = Math.cos(angle) * this.speed;
        this.vy = Math.sin(angle) * this.speed;
        this.color = '#FF4500';
        this.radius = 4;
    }
    
    render(ctx) {
        // Draw arrow shape
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, this.radius * 2);
        gradient.addColorStop(0, '#FFF');
        gradient.addColorStop(0.5, '#FF4500');
        gradient.addColorStop(1, '#FF6500');
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
    }
}

class SpiritBlast extends LightOrb {
    constructor(x, y, angle) {
        super(x, y, angle);
        this.speed = 15;
        this.vx = Math.cos(angle) * this.speed;
        this.vy = Math.sin(angle) * this.speed;
        this.color = '#9932CC';
        this.radius = 3;
        this.life = 90;
        this.maxLife = 90;
    }
    
    render(ctx) {
        // Draw small purple blast
        ctx.fillStyle = this.color;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        
        // Add glow
        ctx.shadowColor = this.color;
        ctx.shadowBlur = 5;
        ctx.fill();
        ctx.shadowBlur = 0;
    }
}

// Sound Manager Class
class SoundManager {
    constructor() {
        this.enabled = true;
        this.sounds = {};
        this.backgroundMusic = null;
        this.volume = 0.7;
        this.musicVolume = 0.3;
        
        // Initialize sounds using Web Audio API with fallbacks
        this.initializeSounds();
    }
    
    initializeSounds() {
        // Create sound effects using synthesized audio
        this.createSoundEffect('shoot', {
            frequency: 800,
            type: 'square',
            duration: 0.1,
            volume: 0.3,
            decay: 0.05
        });
        
        this.createSoundEffect('enemy_defeat', {
            frequency: 1200,
            type: 'sine',
            duration: 0.4,
            volume: 0.4,
            decay: 0.1,
            modulation: { frequency: 400, depth: 0.3 }
        });
        
        this.createSoundEffect('wave_complete', {
            frequency: 600,
            type: 'triangle',
            duration: 1.0,
            volume: 0.5,
            decay: 0.3,
            chord: [600, 800, 1000]
        });
        
        this.createSoundEffect('weapon_pickup', {
            frequency: 1000,
            type: 'sawtooth',
            duration: 0.3,
            volume: 0.4,
            decay: 0.1,
            sweep: { start: 1000, end: 1500 }
        });
        
        this.createSoundEffect('player_hurt', {
            frequency: 300,
            type: 'sawtooth',
            duration: 0.2,
            volume: 0.6,
            decay: 0.05
        });
        
        this.createSoundEffect('thunder_hammer', {
            frequency: 60,
            type: 'sawtooth',
            duration: 0.3,
            volume: 0.8,
            decay: 0.1
        });
        
        this.createSoundEffect('phoenix_bow', {
            frequency: 900,
            type: 'triangle',
            duration: 0.2,
            volume: 0.4,
            decay: 0.05
        });
        
        this.createSoundEffect('spirit_cannon', {
            frequency: 1500,
            type: 'square',
            duration: 0.05,
            volume: 0.3,
            decay: 0.02
        });
        
        // Create background music
        this.createBackgroundMusic();
    }
    
    createSoundEffect(name, config) {
        this.sounds[name] = config;
    }
    
    createBackgroundMusic() {
        // Create a simple heroic melody using oscillators
        this.backgroundMusic = {
            isPlaying: false,
            audioContext: null,
            oscillators: [],
            gainNode: null
        };
    }
    
    playSound(soundName, variations = false) {
        if (!this.enabled || !this.sounds[soundName]) return;
        
        try {
            const audioContext = new (window.AudioContext || window.webkitAudioContext)();
            const config = this.sounds[soundName];
            
            if (config.chord) {
                // Play chord for complex sounds
                config.chord.forEach((freq, index) => {
                    setTimeout(() => {
                        this.playSingleTone(audioContext, freq, config, variations);
                    }, index * 50);
                });
            } else {
                this.playSingleTone(audioContext, config.frequency, config, variations);
            }
        } catch (error) {
            console.warn('Audio not supported:', error);
        }
    }
    
    playSingleTone(audioContext, frequency, config, variations = false) {
        const oscillator = audioContext.createOscillator();
        const gainNode = audioContext.createGain();
        
        // Add slight variation to prevent monotony
        if (variations) {
            frequency *= (0.95 + Math.random() * 0.1);
        }
        
        oscillator.connect(gainNode);
        gainNode.connect(audioContext.destination);
        
        oscillator.type = config.type;
        oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
        
        // Handle frequency sweep
        if (config.sweep) {
            oscillator.frequency.exponentialRampToValueAtTime(
                config.sweep.end, 
                audioContext.currentTime + config.duration * 0.5
            );
        }
        
        // Handle modulation
        if (config.modulation) {
            const lfo = audioContext.createOscillator();
            const modGain = audioContext.createGain();
            
            lfo.frequency.value = config.modulation.frequency;
            modGain.gain.value = config.modulation.depth;
            
            lfo.connect(modGain);
            modGain.connect(oscillator.frequency);
            
            lfo.start();
            lfo.stop(audioContext.currentTime + config.duration);
        }
        
        // Set volume envelope
        const volume = config.volume * this.volume;
        gainNode.gain.setValueAtTime(volume, audioContext.currentTime);
        gainNode.gain.exponentialRampToValueAtTime(0.001, audioContext.currentTime + config.duration);
        
        oscillator.start();
        oscillator.stop(audioContext.currentTime + config.duration);
    }
    
    startBackgroundMusic() {
        if (!this.enabled || this.backgroundMusic.isPlaying) return;
        
        try {
            const audioContext = new (window.AudioContext || window.webkitAudioContext)();
            this.backgroundMusic.audioContext = audioContext;
            this.backgroundMusic.isPlaying = true;
            
            // Create a simple heroic melody loop
            this.playBackgroundLoop(audioContext);
        } catch (error) {
            console.warn('Background music not supported:', error);
        }
    }
    
    playBackgroundLoop(audioContext) {
        if (!this.backgroundMusic.isPlaying) return;
        
        // Heroic melody notes (simplified)
        const melody = [
            { freq: 261.63, duration: 0.5 }, // C4
            { freq: 329.63, duration: 0.5 }, // E4
            { freq: 392.00, duration: 0.5 }, // G4
            { freq: 523.25, duration: 0.5 }, // C5
            { freq: 392.00, duration: 0.5 }, // G4
            { freq: 329.63, duration: 0.5 }, // E4
            { freq: 261.63, duration: 1.0 }, // C4
        ];
        
        let currentTime = audioContext.currentTime;
        
        melody.forEach((note, index) => {
            const oscillator = audioContext.createOscillator();
            const gainNode = audioContext.createGain();
            
            oscillator.connect(gainNode);
            gainNode.connect(audioContext.destination);
            
            oscillator.type = 'triangle';
            oscillator.frequency.setValueAtTime(note.freq, currentTime);
            
            const volume = 0.1 * this.musicVolume;
            gainNode.gain.setValueAtTime(0, currentTime);
            gainNode.gain.linearRampToValueAtTime(volume, currentTime + 0.05);
            gainNode.gain.linearRampToValueAtTime(volume * 0.7, currentTime + note.duration * 0.8);
            gainNode.gain.linearRampToValueAtTime(0, currentTime + note.duration);
            
            oscillator.start(currentTime);
            oscillator.stop(currentTime + note.duration);
            
            currentTime += note.duration;
        });
        
        // Schedule next loop
        const totalDuration = melody.reduce((sum, note) => sum + note.duration, 0);
        setTimeout(() => {
            if (this.backgroundMusic.isPlaying) {
                this.playBackgroundLoop(audioContext);
            }
        }, totalDuration * 1000 + 1000); // 1 second pause between loops
    }
    
    stopBackgroundMusic() {
        if (this.backgroundMusic.audioContext) {
            this.backgroundMusic.isPlaying = false;
            // Note: AudioContext will clean up automatically
        }
    }
    
    toggleMute() {
        this.enabled = !this.enabled;
        if (!this.enabled) {
            this.stopBackgroundMusic();
        }
        return this.enabled;
    }
    
    setVolume(volume) {
        this.volume = Math.max(0, Math.min(1, volume));
    }
    
    setMusicVolume(volume) {
        this.musicVolume = Math.max(0, Math.min(1, volume));
    }
}

// Store reference to game instance for weapon access
let game;

// Initialize the game when the page loads
window.addEventListener('load', () => {
    game = new Game();
});