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
        this.collectedWeapons = []; // Array to store all collected weapons
        this.currentWeaponIndex = 0; // Index of currently active weapon
        this.activeWeapon = null; // Current active weapon (for compatibility)
        this.soundManager = new SoundManager();
        this.highScoreManager = new HighScoreManager();
        
        // Epic Arena Elements
        this.healingPillar = {
            x: this.width / 2,
            y: this.height / 2,
            radius: 50, // Larger for bigger map
            isActive: false,
            cooldown: 0,
            pulseTimer: 0
        };
        this.stoneBridges = [];
        this.arenaDecorations = [];
        
        // Screen shake system
        this.screenShake = {
            x: 0,
            y: 0,
            intensity: 0,
            duration: 0
        };
        
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
        this.enemySpawnDelay = 15; // Reduced by 50% - more aggressive spawning
        this.lastSpawnTime = 0;
        
        this.setupWeaponShrines();
        this.setupArenaElements();
        this.setupCanvas();
        this.setupEventListeners();
        if (this.isMobile) {
            this.setupMobileControls();
        }
        
        // Initialize high scores display
        this.updateHighScoresDisplay();
        
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
        // Create 4 weapon shrines at temple ruins in corners
        const margin = 80; // Distance from edges for temple placement
        const weaponTypes = ['thunder_hammer', 'phoenix_bow', 'shield_of_light', 'spirit_cannon'];
        
        // Top-left temple ruins
        this.weaponShrines.push(new WeaponShrine(margin, margin, weaponTypes[0]));
        // Top-right temple ruins
        this.weaponShrines.push(new WeaponShrine(this.width - margin, margin, weaponTypes[1]));
        // Bottom-left temple ruins
        this.weaponShrines.push(new WeaponShrine(margin, this.height - margin, weaponTypes[2]));
        // Bottom-right temple ruins
        this.weaponShrines.push(new WeaponShrine(this.width - margin, this.height - margin, weaponTypes[3]));
    }
    
    setupArenaElements() {
        // Create stone bridges connecting corners to center (scaled for larger map)
        const centerX = this.width / 2;
        const centerY = this.height / 2;
        const bridgeLength = 160; // Increased for larger map
        const bridgeWidth = 25;
        
        // Four stone bridges from corners toward center
        this.stoneBridges = [
            // Top-left to center
            { x: centerX - 80, y: centerY - 80, width: bridgeLength, height: bridgeWidth, rotation: -Math.PI/4, destroyed: false },
            // Top-right to center  
            { x: centerX + 80, y: centerY - 80, width: bridgeLength, height: bridgeWidth, rotation: Math.PI/4, destroyed: false },
            // Bottom-left to center
            { x: centerX - 80, y: centerY + 80, width: bridgeLength, height: bridgeWidth, rotation: Math.PI/4, destroyed: false },
            // Bottom-right to center
            { x: centerX + 80, y: centerY + 80, width: bridgeLength, height: bridgeWidth, rotation: -Math.PI/4, destroyed: false }
        ];
        
        // Arena decorations (temple ruins) - scaled for larger map
        this.arenaDecorations = [
            // Ancient pillars around the arena
            { type: 'pillar', x: 150, y: 150, width: 30, height: 80 },
            { type: 'pillar', x: this.width - 150, y: 150, width: 30, height: 80 },
            { type: 'pillar', x: 150, y: this.height - 150, width: 30, height: 80 },
            { type: 'pillar', x: this.width - 150, y: this.height - 150, width: 30, height: 80 },
            
            // Broken walls
            { type: 'wall', x: 300, y: 80, width: 120, height: 25 },
            { type: 'wall', x: this.width - 420, y: 80, width: 120, height: 25 },
            { type: 'wall', x: 80, y: 250, width: 25, height: 120 },
            { type: 'wall', x: this.width - 70, y: 200, width: 20, height: 80 }
        ];
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
                this.activeWeapon.attack(this.player, centerX, centerY, this.projectiles, this.particles, this);
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
            this.activeWeapon.attack(this.player, centerX, centerY, this.projectiles, this.particles, this);
        }
    }
    
    setupEventListeners() {
        // Keyboard events
        document.addEventListener('keydown', (e) => {
            this.keys[e.code] = true;
            
            // Weapon switching with number keys (1-4)
            if (this.gameState === 'playing' && this.collectedWeapons.length > 0) {
                const weaponKeys = ['Digit1', 'Digit2', 'Digit3', 'Digit4'];
                const keyIndex = weaponKeys.indexOf(e.code);
                
                if (keyIndex !== -1 && keyIndex < this.collectedWeapons.length) {
                    this.switchToWeapon(keyIndex);
                }
                
                // Also allow switching with Tab key
                if (e.code === 'Tab') {
                    e.preventDefault(); // Prevent default tab behavior
                    this.switchToNextWeapon();
                }
            }
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
                    this.activeWeapon.attack(this.player, this.mouse.x, this.mouse.y, this.projectiles, this.particles, this);
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
        
        // High score submission
        document.getElementById('submit-score-btn').addEventListener('click', () => {
            this.submitHighScore();
        });
        
        // Allow Enter key to submit high score
        document.getElementById('player-name-input').addEventListener('keypress', (e) => {
            if (e.key === 'Enter') {
                this.submitHighScore();
            }
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
        this.enemySpawnDelay = 15;
        this.lastSpawnTime = 0;
        
        // Reset weapon system
        this.collectedWeapons = [];
        this.currentWeaponIndex = 0;
        this.activeWeapon = null;
        this.weaponShrines.forEach(shrine => shrine.reset());
        this.updateWeaponDisplay();
        
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
                // Check if this weapon type is already collected
                const weaponType = shrine.weaponType;
                const alreadyHave = this.collectedWeapons.some(weapon => weapon.type === weaponType);
                
                if (!alreadyHave) {
                    // Pick up new weapon and add to collection
                    const newWeapon = shrine.pickupWeapon();
                    newWeapon.type = weaponType; // Store weapon type for identification
                    this.collectedWeapons.push(newWeapon);
                    
                    // Set as active weapon if it's the first weapon collected
                    if (this.collectedWeapons.length === 1) {
                        this.currentWeaponIndex = 0;
                        this.activeWeapon = this.collectedWeapons[0];
                    }
                    
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
                    
                    // Update UI to show collected weapons
                    this.updateWeaponDisplay();
                }
                break;
            }
        }
    }
    
    // Weapon management methods
    switchToWeapon(index) {
        if (index >= 0 && index < this.collectedWeapons.length) {
            this.currentWeaponIndex = index;
            this.activeWeapon = this.collectedWeapons[index];
            this.updateWeaponDisplay();
            
            // Play weapon switch sound
            this.soundManager.playSound('weapon_switch');
        }
    }
    
    switchToNextWeapon() {
        if (this.collectedWeapons.length > 1) {
            this.currentWeaponIndex = (this.currentWeaponIndex + 1) % this.collectedWeapons.length;
            this.activeWeapon = this.collectedWeapons[this.currentWeaponIndex];
            this.updateWeaponDisplay();
            
            // Play weapon switch sound
            this.soundManager.playSound('weapon_switch');
        }
    }
    
    updateWeaponDisplay() {
        const weaponDisplay = document.getElementById('weapon-display');
        const activeWeaponSpan = document.getElementById('active-weapon');
        
        if (this.collectedWeapons.length > 0) {
            // Show weapon display
            weaponDisplay.style.display = 'block';
            
            // Create weapon list display
            let weaponText = '';
            this.collectedWeapons.forEach((weapon, index) => {
                const weaponName = this.getWeaponName(weapon.type);
                const keyNumber = index + 1;
                const isActive = index === this.currentWeaponIndex;
                
                if (isActive) {
                    weaponText += `[${keyNumber}] ${weaponName} (ACTIVE) `;
                } else {
                    weaponText += `[${keyNumber}] ${weaponName} `;
                }
            });
            
            activeWeaponSpan.textContent = weaponText;
        } else {
            // Hide weapon display if no weapons
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
    
    spawnEnemyFromEdge() {
        // Spawn diverse enemy types based on wave progression
        let x, y;
        const side = Math.floor(Math.random() * 4);
        const margin = 25; // Closer to screen edge - more aggressive spawning
        
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
        
        // Determine enemy type based on wave progression
        let enemyToSpawn;
        
        if (this.wave <= 2) {
            // Wave 1-2: 8-12 Shadow Crawlers (fast, weak)
            enemyToSpawn = new ShadowCrawler(x, y, this.wave);
        } else if (this.wave <= 5) {
            // Wave 3-5: Mix of Shadow Crawlers and Stone Demons (slow, tanky, 3 hits)
            const enemyTypes = ['crawler', 'stone'];
            const weights = this.wave === 3 ? [0.7, 0.3] : this.wave === 4 ? [0.5, 0.5] : [0.3, 0.7];
            const randomValue = Math.random();
            
            if (randomValue < weights[0]) {
                enemyToSpawn = new ShadowCrawler(x, y, this.wave);
            } else {
                enemyToSpawn = new StoneDemon(x, y, this.wave);
            }
        } else if (this.wave <= 8) {
            // Wave 6-8: Mix of all types, add Flying Wraiths (sine wave movement)
            const enemyTypes = ['crawler', 'stone', 'wraith'];
            const weights = this.wave === 6 ? [0.4, 0.4, 0.2] : this.wave === 7 ? [0.3, 0.4, 0.3] : [0.2, 0.3, 0.5];
            const randomValue = Math.random();
            
            if (randomValue < weights[0]) {
                enemyToSpawn = new ShadowCrawler(x, y, this.wave);
            } else if (randomValue < weights[0] + weights[1]) {
                enemyToSpawn = new StoneDemon(x, y, this.wave);
            } else {
                enemyToSpawn = new FlyingWraith(x, y, this.wave);
            }
        } else {
            // Wave 9-10: Shadow Giant boss + swarms - spawn Shadow Giant first
            if (this.wave >= 9 && this.enemiesSpawned === 0) {
                // First spawn in wave 9+ is always the Shadow Giant
                enemyToSpawn = new ShadowGiant(x, y, this.wave);
            } else {
                // Mix of support enemies for the boss fight
                const enemyTypes = ['crawler', 'stone', 'wraith'];
                const weights = [0.5, 0.25, 0.25]; // More crawlers to create swarm feel
                const randomValue = Math.random();
                
                if (randomValue < weights[0]) {
                    enemyToSpawn = new ShadowCrawler(x, y, this.wave);
                } else if (randomValue < weights[0] + weights[1]) {
                    enemyToSpawn = new StoneDemon(x, y, this.wave);
                } else {
                    enemyToSpawn = new FlyingWraith(x, y, this.wave);
                }
            }
        }
        
        this.enemies.push(enemyToSpawn);
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
        this.enemySpawnDelay = Math.max(8, this.enemySpawnDelay - 1); // Much faster spawn rate (min 8 frames)
        
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
                    
                    let enemyDefeated = false;
                    
                    // Handle different enemy damage systems
                    if (this.enemies[j].takeDamage) {
                        // Enemy has takeDamage method (Stone Demon, Shadow Giant)
                        enemyDefeated = this.enemies[j].takeDamage();
                    } else {
                        // Standard enemy (Shadow Crawler, Flying Wraith, old Shadow Demon)
                        enemyDefeated = true;
                    }
                    
                    // Add screen shake on every projectile hit
                    this.addScreenShake(3, 8);
                    
                    // Remove projectile regardless of enemy defeat
                    this.projectiles.splice(i, 1);
                    
                    if (enemyDefeated) {
                        // Add combat momentum to player
                        this.player.addCombatMomentum();
                        // Create sparkle effect when enemy is defeated
                        this.createSparkleEffect(this.enemies[j].x, this.enemies[j].y);
                        
                        // Play enemy defeat sound
                        this.soundManager.playSound('enemy_defeat', true);
                        
                        // Create explosion particles that drop toward player
                        const toPlayerAngle = Math.atan2(this.player.y - this.enemies[j].y, this.player.x - this.enemies[j].x);
                        for (let k = 0; k < 8; k++) {
                            const angleVariation = (Math.random() - 0.5) * 1.5; // ±0.75 radians variation
                            const particleAngle = toPlayerAngle + angleVariation;
                            const speed = 2 + Math.random() * 3;
                            
                            const particle = new Particle(this.enemies[j].x, this.enemies[j].y, 'explosion');
                            particle.vx = Math.cos(particleAngle) * speed;
                            particle.vy = Math.sin(particleAngle) * speed;
                            this.particles.push(particle);
                        }
                        
                        // Award points based on enemy type
                        let points = 10 * this.wave;
                        if (this.enemies[j].type === 'stone_demon') {
                            points *= 3; // Bonus for tanky enemies
                        } else if (this.enemies[j].type === 'shadow_giant') {
                            points *= 5; // Big bonus for boss
                        }
                        this.score += points;
                        
                        // Remove defeated enemy
                        this.enemies.splice(j, 1);
                        
                        // Add screen shake for boss defeats
                        if (this.enemies[j]?.type === 'shadow_giant') {
                            this.addScreenShake(15, 30);
                        }
                    } else {
                        // Enemy took damage but wasn't defeated - add hit particles
                        for (let k = 0; k < 3; k++) {
                            this.particles.push(new Particle(
                                this.enemies[j].x + (Math.random() - 0.5) * 20, 
                                this.enemies[j].y + (Math.random() - 0.5) * 20, 
                                'light'
                            ));
                        }
                    }
                    
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
        
        // Check for new high score
        const isNewHighScore = this.highScoreManager.isNewHighScore(this.score, this.wave);
        
        if (isNewHighScore) {
            this.showNewHighScorePrompt();
        } else {
            document.getElementById('game-over').style.display = 'block';
        }
        
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
    
    showNewHighScorePrompt() {
        // Hide normal game over screen
        document.getElementById('game-over').style.display = 'none';
        
        // Show high score entry screen
        const highScoreScreen = document.getElementById('high-score-entry');
        if (highScoreScreen) {
            highScoreScreen.style.display = 'block';
            
            // Update the score display in the high score screen
            const hsScoreElement = document.getElementById('hs-final-score');
            if (hsScoreElement) {
                hsScoreElement.textContent = this.score;
            }
            
            // Focus on name input
            const nameInput = document.getElementById('player-name-input');
            if (nameInput) {
                nameInput.value = ''; // Clear any previous input
                nameInput.focus();
                nameInput.select();
            }
            
            // Show celebration animation
            this.showHighScoreCelebration();
        }
    }
    
    showHighScoreCelebration() {
        // Create celebration effect
        const celebration = document.createElement('div');
        celebration.className = 'high-score-celebration';
        celebration.innerHTML = `
            <div class="celebration-text">NEW HIGH SCORE!</div>
            <div class="celebration-score">Score: ${this.score}</div>
            <div class="celebration-wave">Wave: ${this.wave}</div>
        `;
        document.body.appendChild(celebration);
        
        // Remove after animation
        setTimeout(() => {
            if (document.body.contains(celebration)) {
                document.body.removeChild(celebration);
            }
        }, 3000);
        
        // Play celebration sound
        this.soundManager.playSound('high_score_celebration');
    }
    
    submitHighScore() {
        const nameInput = document.getElementById('player-name-input');
        const playerName = nameInput ? nameInput.value.trim() || 'Anonymous' : 'Anonymous';
        
        // Add the score
        const rank = this.highScoreManager.addScore(playerName, this.score, this.wave);
        
        // Hide high score entry screen
        const highScoreScreen = document.getElementById('high-score-entry');
        if (highScoreScreen) {
            highScoreScreen.style.display = 'none';
        }
        
        // Show normal game over screen with rank info
        document.getElementById('game-over').style.display = 'block';
        
        // Update game over message to show rank
        const rankMessage = document.getElementById('high-score-rank');
        if (rankMessage) {
            rankMessage.textContent = `#${rank} on the leaderboard!`;
            rankMessage.style.display = 'block';
        }
        
        // Update high scores display
        this.updateHighScoresDisplay();
    }
    
    addScreenShake(intensity, duration) {
        this.screenShake.intensity = Math.max(this.screenShake.intensity, intensity);
        this.screenShake.duration = Math.max(this.screenShake.duration, duration);
    }
    
    updateScreenShake() {
        if (this.screenShake.duration > 0) {
            this.screenShake.duration--;
            const shakeAmount = this.screenShake.intensity * (this.screenShake.duration / 30);
            this.screenShake.x = (Math.random() - 0.5) * shakeAmount;
            this.screenShake.y = (Math.random() - 0.5) * shakeAmount;
        } else {
            this.screenShake.x = 0;
            this.screenShake.y = 0;
        }
    }
    
    updateHealingPillar() {
        this.healingPillar.pulseTimer += 0.1;
        
        // Activate healing pillar during wave intermissions
        if (this.waveState === 'intermission') {
            this.healingPillar.isActive = true;
            this.healingPillar.cooldown = 0;
            
            // Check if player is near pillar for healing
            const dx = this.player.x - this.healingPillar.x;
            const dy = this.player.y - this.healingPillar.y;
            const distance = Math.sqrt(dx * dx + dy * dy);
            
            if (distance < this.healingPillar.radius + this.player.radius && this.healingPillar.cooldown <= 0) {
                // Heal the player
                const healAmount = 2; // Heal per frame while near pillar
                this.player.heal(healAmount);
                this.healingPillar.cooldown = 5; // Small cooldown between heals
                
                // Create healing particles
                if (Math.random() < 0.3) {
                    this.particles.push(new Particle(
                        this.healingPillar.x + (Math.random() - 0.5) * 60,
                        this.healingPillar.y + (Math.random() - 0.5) * 60,
                        'healing'
                    ));
                }
            }
        } else {
            this.healingPillar.isActive = false;
        }
        
        if (this.healingPillar.cooldown > 0) {
            this.healingPillar.cooldown--;
        }
    }
    
    updateHighScoresDisplay() {
        const highScoresList = document.getElementById('high-scores-list');
        if (!highScoresList) return;
        
        const scores = this.highScoreManager.getTopScores();
        
        if (scores.length === 0) {
            highScoresList.innerHTML = '<div class="no-scores">No high scores yet!</div>';
            return;
        }
        
        let html = '';
        scores.forEach((score, index) => {
            const rank = index + 1;
            html += `
                <div class="score-entry ${rank === 1 ? 'first-place' : ''}">
                    <span class="rank">#${rank}</span>
                    <span class="name">${score.playerName}</span>
                    <span class="wave">Wave ${score.wave}</span>
                    <span class="score">${score.score.toLocaleString()}</span>
                </div>
            `;
        });
        
        highScoresList.innerHTML = html;
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
            // Shadow Giants need game instance for swarm spawning
            if (enemy.type === 'shadow_giant') {
                enemy.update(this.player.x, this.player.y, this);
            } else {
                enemy.update(this.player.x, this.player.y);
            }
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
        
        // Draw sacred circle in center (scaled for larger map)
        this.ctx.strokeStyle = '#FFD700';
        this.ctx.lineWidth = 4;
        this.ctx.beginPath();
        this.ctx.arc(this.width / 2, this.height / 2, 140, 0, Math.PI * 2);
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
        this.maxSpeed = 12; // Tripled from 4 to 12
        this.baseAcceleration = 0.4; // Slightly increased base acceleration
        this.maxAcceleration = 0.8; // Higher acceleration at peak
        this.friction = 0.82; // Slightly less friction for momentum
        this.color = '#FFD700';
        this.attackCooldown = 0;
        this.health = 100;
        this.maxHealth = 100;
        this.lightEnergy = 100;
        this.maxLightEnergy = 100;
        
        // Velocity components for smooth movement
        this.vx = 0;
        this.vy = 0;
        
        // Momentum-based movement variables
        this.currentSpeed = 0;
        this.accelerationCurve = 0; // Build up over time
        
        // Dash mechanics
        this.dashCooldown = 0;
        this.dashDuration = 8; // Frames the dash lasts
        this.dashSpeed = 36; // 3x max speed
        this.isDashing = false;
        this.dashFramesLeft = 0;
        
        // Combat momentum system
        this.combatMomentum = 0; // Frames of momentum left
        this.combatMomentumDuration = 120; // 2 seconds at 60fps
        this.combatSpeedBonus = 0.2; // 20% speed bonus
        
        // Collision box (circular)
        this.collisionRadius = this.radius;
        
        // Visual effects
        this.lightPulse = 0;
        this.energyRegenRate = 0.5;
    }
    
    update(keys, canvasWidth, canvasHeight, virtualJoystick = null) {
        // Handle dash cooldown
        if (this.dashCooldown > 0) this.dashCooldown--;
        if (this.dashFramesLeft > 0) this.dashFramesLeft--;
        if (this.dashFramesLeft <= 0) this.isDashing = false;
        
        // Handle combat momentum decay
        if (this.combatMomentum > 0) this.combatMomentum--;
        
        // Handle input and apply acceleration
        let inputX = 0;
        let inputY = 0;
        let dashRequested = false;
        
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
            
            // Check for dash input (SHIFT key)
            if ((keys['ShiftLeft'] || keys['ShiftRight']) && this.dashCooldown <= 0 && (inputX !== 0 || inputY !== 0)) {
                dashRequested = true;
            }
        }
        
        // Normalize diagonal movement
        if (inputX !== 0 && inputY !== 0) {
            inputX *= 0.707; // 1/sqrt(2)
            inputY *= 0.707;
        }
        
        // Calculate current movement speed
        this.currentSpeed = Math.sqrt(this.vx * this.vx + this.vy * this.vy);
        
        // Handle dash mechanics
        if (dashRequested && !this.isDashing) {
            this.isDashing = true;
            this.dashFramesLeft = this.dashDuration;
            this.dashCooldown = 20; // 20 frame cooldown
            
            // Set dash velocity
            if (inputX !== 0 || inputY !== 0) {
                this.vx = inputX * this.dashSpeed;
                this.vy = inputY * this.dashSpeed;
            }
        }
        
        // Apply movement (different logic for dashing vs normal)
        if (this.isDashing) {
            // During dash, maintain velocity with slight decay
            this.vx *= 0.95;
            this.vy *= 0.95;
        } else {
            // Momentum-based acceleration curves
            if (inputX !== 0 || inputY !== 0) {
                // Build acceleration curve over time
                this.accelerationCurve = Math.min(this.accelerationCurve + 0.05, 1.0);
            } else {
                // Reset acceleration curve when not moving
                this.accelerationCurve *= 0.9;
            }
            
            // Calculate dynamic acceleration (slow start, fast sprint)
            const dynamicAcceleration = this.baseAcceleration + 
                (this.maxAcceleration - this.baseAcceleration) * this.accelerationCurve;
            
            // Apply acceleration based on input
            this.vx += inputX * dynamicAcceleration;
            this.vy += inputY * dynamicAcceleration;
            
            // Apply friction when no input
            if (inputX === 0) this.vx *= this.friction;
            if (inputY === 0) this.vy *= this.friction;
            
            // Calculate effective max speed (with combat momentum bonus)
            let effectiveMaxSpeed = this.maxSpeed;
            if (this.combatMomentum > 0) {
                effectiveMaxSpeed *= (1 + this.combatSpeedBonus);
            }
            
            // Limit maximum speed
            const speed = Math.sqrt(this.vx * this.vx + this.vy * this.vy);
            if (speed > effectiveMaxSpeed) {
                this.vx = (this.vx / speed) * effectiveMaxSpeed;
                this.vy = (this.vy / speed) * effectiveMaxSpeed;
            }
        }
        
        // Update position
        this.x += this.vx;
        this.y += this.vy;
        
        // Keep player within bounds with proper collision
        if (this.x - this.collisionRadius < 0) {
            this.x = this.collisionRadius;
            this.vx = Math.abs(this.vx) * 0.5; // Bounce with energy loss
        }
        if (this.x + this.collisionRadius > canvasWidth) {
            this.x = canvasWidth - this.collisionRadius;
            this.vx = -Math.abs(this.vx) * 0.5;
        }
        if (this.y - this.collisionRadius < 0) {
            this.y = this.collisionRadius;
            this.vy = Math.abs(this.vy) * 0.5;
        }
        if (this.y + this.collisionRadius > canvasHeight) {
            this.y = canvasHeight - this.collisionRadius;
            this.vy = -Math.abs(this.vy) * 0.5;
        }
        
        // Update attack cooldown (reduced to 6 frames from 12)
        if (this.attackCooldown > 0) this.attackCooldown--;
        
        // Regenerate light energy over time
        if (this.lightEnergy < this.maxLightEnergy) {
            this.lightEnergy = Math.min(this.maxLightEnergy, this.lightEnergy + this.energyRegenRate);
        }
        
        // Update visual effects
        this.lightPulse += 0.15;
    }
    
    // Add combat momentum when player gets a kill
    addCombatMomentum() {
        this.combatMomentum = this.combatMomentumDuration;
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
        
        // Create improved light orb projectile with increased speed
        projectiles.push(new LightOrb(this.x, this.y, angle));
        this.attackCooldown = 6; // Reduced from 12 to 6 for faster attacks
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
        this.speed = 1.4 + (wave - 1) * 0.16; // Doubled speed - aggressive movement
        this.color = '#4A0E4E';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 1; // Basic shadow demons have 1 health
        this.type = 'shadow_demon';
    }
    
    update(playerX, playerY) {
        // Move directly towards player - aggressive, no wandering
        const angle = Math.atan2(playerY - this.y, playerX - this.x);
        this.x += Math.cos(angle) * this.speed;
        this.y += Math.sin(angle) * this.speed;
        
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

// Shadow Crawler - Fast, weak enemies for waves 1-2
class ShadowCrawler {
    constructor(x, y, wave) {
        this.x = x;
        this.y = y;
        this.radius = 8 + Math.min(wave - 1, 4); // Smaller than ShadowDemons
        this.speed = 2.4 + (wave - 1) * 0.2; // Doubled speed - very fast aggressive movement
        this.color = '#2E1A47';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 1;
        this.type = 'shadow_crawler';
        this.scrambleTimer = Math.random() * 60; // Erratic movement timer
    }
    
    update(playerX, playerY) {
        this.scrambleTimer++;
        
        // Erratic, scrambling movement toward player
        const baseAngle = Math.atan2(playerY - this.y, playerX - this.x);
        const scramble = Math.sin(this.scrambleTimer * 0.3) * 0.8; // More erratic than wobble
        const finalAngle = baseAngle + scramble;
        
        this.x += Math.cos(finalAngle) * this.speed;
        this.y += Math.sin(finalAngle) * this.speed;
        
        this.shadowPulse += 0.15; // Faster pulse than ShadowDemon
    }
    
    render(ctx) {
        const pulseIntensity = 0.5 + Math.sin(this.shadowPulse) * 0.4;
        const shadowRadius = this.radius * (1.8 + pulseIntensity * 0.3);
        
        // Lighter shadow aura (crawlers are weaker)
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, shadowRadius);
        gradient.addColorStop(0, `rgba(46, 26, 71, ${0.6 + pulseIntensity * 0.2})`);
        gradient.addColorStop(1, 'rgba(46, 26, 71, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, shadowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw crawler body (more insect-like)
        ctx.fillStyle = this.color;
        ctx.strokeStyle = '#1A0F2A';
        ctx.lineWidth = 1.5;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw smaller, more numerous eyes (spider-like)
        const eyeSize = Math.max(1, this.radius / 8);
        ctx.fillStyle = '#FF4444';
        for (let i = 0; i < 6; i++) {
            const eyeAngle = (Math.PI * 2 / 6) * i;
            const eyeX = this.x + Math.cos(eyeAngle) * this.radius * 0.5;
            const eyeY = this.y + Math.sin(eyeAngle) * this.radius * 0.5;
            ctx.beginPath();
            ctx.arc(eyeX, eyeY, eyeSize, 0, Math.PI * 2);
            ctx.fill();
        }
        
        // Draw scuttle legs
        ctx.strokeStyle = this.color;
        ctx.lineWidth = 2;
        ctx.lineCap = 'round';
        for (let i = 0; i < 8; i++) {
            const legAngle = (Math.PI * 2 / 8) * i + this.shadowPulse * 0.3;
            const legLength = this.radius * 0.8;
            const startX = this.x + Math.cos(legAngle) * this.radius * 0.7;
            const startY = this.y + Math.sin(legAngle) * this.radius * 0.7;
            const endX = startX + Math.cos(legAngle) * legLength;
            const endY = startY + Math.sin(legAngle) * legLength;
            
            ctx.beginPath();
            ctx.moveTo(startX, startY);
            ctx.lineTo(endX, endY);
            ctx.stroke();
        }
    }
}

// Stone Demon - Slow, tanky enemies for waves 3-5
class StoneDemon {
    constructor(x, y, wave) {
        this.x = x;
        this.y = y;
        this.radius = 18 + Math.min(wave - 3, 6); // Larger than others
        this.speed = 0.8 + (wave - 3) * 0.1; // Doubled speed - still slow but more aggressive
        this.color = '#8B4513';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 3; // Takes 3 hits to destroy
        this.maxHealth = 3;
        this.type = 'stone_demon';
        this.crackPattern = Math.random();
    }
    
    update(playerX, playerY) {
        // Slow, methodical movement toward player
        const angle = Math.atan2(playerY - this.y, playerX - this.x);
        this.x += Math.cos(angle) * this.speed;
        this.y += Math.sin(angle) * this.speed;
        
        this.shadowPulse += 0.05; // Very slow pulse
    }
    
    takeDamage() {
        this.health--;
        return this.health <= 0;
    }
    
    render(ctx) {
        const pulseIntensity = 0.2 + Math.sin(this.shadowPulse) * 0.1;
        const shadowRadius = this.radius * (1.5 + pulseIntensity * 0.2);
        
        // Stone-like shadow aura
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, shadowRadius);
        gradient.addColorStop(0, `rgba(139, 69, 19, ${0.4 + pulseIntensity * 0.1})`);
        gradient.addColorStop(1, 'rgba(139, 69, 19, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, shadowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw stone body with health-based cracking
        const healthRatio = this.health / this.maxHealth;
        const stoneColor = healthRatio > 0.6 ? '#8B4513' : healthRatio > 0.3 ? '#A0522D' : '#CD853F';
        
        ctx.fillStyle = stoneColor;
        ctx.strokeStyle = '#654321';
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw cracks based on damage taken
        if (this.health < this.maxHealth) {
            ctx.strokeStyle = '#2F1B14';
            ctx.lineWidth = 2;
            ctx.lineCap = 'round';
            
            const cracksToShow = this.maxHealth - this.health;
            for (let i = 0; i < cracksToShow; i++) {
                const crackAngle = (this.crackPattern + i) * Math.PI * 2 / 3;
                const crackLength = this.radius * 0.8;
                const startX = this.x + Math.cos(crackAngle) * this.radius * 0.3;
                const startY = this.y + Math.sin(crackAngle) * this.radius * 0.3;
                const endX = startX + Math.cos(crackAngle) * crackLength;
                const endY = startY + Math.sin(crackAngle) * crackLength;
                
                ctx.beginPath();
                ctx.moveTo(startX, startY);
                ctx.lineTo(endX, endY);
                ctx.stroke();
            }
        }
        
        // Draw glowing stone eyes
        const eyeSize = Math.max(3, this.radius / 5);
        ctx.fillStyle = '#FF6600';
        ctx.beginPath();
        ctx.arc(this.x - this.radius * 0.35, this.y - this.radius * 0.25, eyeSize, 0, Math.PI * 2);
        ctx.arc(this.x + this.radius * 0.35, this.y - this.radius * 0.25, eyeSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw stone runes
        ctx.strokeStyle = '#FF6600';
        ctx.lineWidth = 1.5;
        ctx.lineCap = 'round';
        const runeSize = this.radius * 0.3;
        ctx.beginPath();
        ctx.moveTo(this.x - runeSize/2, this.y + this.radius * 0.3);
        ctx.lineTo(this.x + runeSize/2, this.y + this.radius * 0.3);
        ctx.moveTo(this.x, this.y + this.radius * 0.1);
        ctx.lineTo(this.x, this.y + this.radius * 0.5);
        ctx.stroke();
    }
}

// Flying Wraith - Sine wave movement for waves 6-8
class FlyingWraith {
    constructor(x, y, wave) {
        this.x = x;
        this.y = y;
        this.startY = y;
        this.radius = 10 + Math.min(wave - 6, 4);
        this.speed = 1.6 + (wave - 6) * 0.16; // Doubled speed - fast ethereal movement
        this.color = '#9370DB';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 1;
        this.type = 'flying_wraith';
        this.sineTimer = Math.random() * Math.PI * 2;
        this.sineAmplitude = 60 + wave * 5;
        this.baseAngle = 0;
        this.floatOffset = Math.random() * Math.PI * 2;
    }
    
    update(playerX, playerY) {
        // Calculate base movement toward player
        this.baseAngle = Math.atan2(playerY - this.y, playerX - this.x);
        
        // Move toward player with sine wave pattern
        const baseX = this.x + Math.cos(this.baseAngle) * this.speed;
        const baseY = this.y + Math.sin(this.baseAngle) * this.speed;
        
        // Add sine wave movement perpendicular to base direction
        this.sineTimer += 0.12;
        const perpAngle = this.baseAngle + Math.PI / 2;
        const sineOffset = Math.sin(this.sineTimer) * this.sineAmplitude * 0.02;
        
        this.x = baseX + Math.cos(perpAngle) * sineOffset;
        this.y = baseY + Math.sin(perpAngle) * sineOffset;
        
        // Add floating effect
        this.floatOffset += 0.08;
        this.y += Math.sin(this.floatOffset) * 0.5;
        
        this.shadowPulse += 0.12;
    }
    
    render(ctx) {
        const pulseIntensity = 0.6 + Math.sin(this.shadowPulse) * 0.4;
        const shadowRadius = this.radius * (2.5 + pulseIntensity * 0.5);
        
        // Ethereal wraith aura
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, shadowRadius);
        gradient.addColorStop(0, `rgba(147, 112, 219, ${0.7 + pulseIntensity * 0.3})`);
        gradient.addColorStop(0.5, `rgba(147, 112, 219, ${0.3 + pulseIntensity * 0.2})`);
        gradient.addColorStop(1, 'rgba(147, 112, 219, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, shadowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw wraith body (semi-transparent)
        ctx.fillStyle = `rgba(147, 112, 219, ${0.8 + pulseIntensity * 0.2})`;
        ctx.strokeStyle = `rgba(75, 0, 130, ${0.9})`;
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw ethereal eyes
        const eyeSize = Math.max(2, this.radius / 4);
        ctx.fillStyle = '#E6E6FA';
        ctx.beginPath();
        ctx.arc(this.x - this.radius * 0.3, this.y - this.radius * 0.2, eyeSize, 0, Math.PI * 2);
        ctx.arc(this.x + this.radius * 0.3, this.y - this.radius * 0.2, eyeSize, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw wispy trails
        ctx.strokeStyle = `rgba(147, 112, 219, ${0.6 + pulseIntensity * 0.2})`;
        ctx.lineWidth = 2;
        ctx.lineCap = 'round';
        
        for (let i = 0; i < 6; i++) {
            const trailAngle = (Math.PI * 2 / 6) * i + this.shadowPulse * 0.5;
            const trailLength = this.radius * (1.2 + Math.sin(this.shadowPulse + i) * 0.3);
            const startX = this.x + Math.cos(trailAngle) * this.radius * 0.8;
            const startY = this.y + Math.sin(trailAngle) * this.radius * 0.8;
            const endX = startX + Math.cos(trailAngle) * trailLength;
            const endY = startY + Math.sin(trailAngle) * trailLength;
            
            ctx.beginPath();
            ctx.moveTo(startX, startY);
            ctx.lineTo(endX, endY);
            ctx.stroke();
        }
    }
}

// Shadow Giant Boss - Appears on waves 9-10 with swarms
class ShadowGiant {
    constructor(x, y, wave) {
        this.x = x;
        this.y = y;
        this.radius = 35 + Math.min(wave - 9, 8); // Much larger
        this.speed = 0.6 + (wave - 9) * 0.06; // Doubled speed - slow but relentless boss
        this.color = '#2E0A33';
        this.wave = wave;
        this.shadowPulse = Math.random() * Math.PI * 2;
        this.health = 8 + (wave - 9) * 2; // Very tanky boss
        this.maxHealth = this.health;
        this.type = 'shadow_giant';
        this.swarmSpawnTimer = 0;
        this.swarmSpawnCooldown = 300; // 5 seconds at 60fps
        this.tentacles = [];
        
        // Initialize tentacles
        for (let i = 0; i < 8; i++) {
            this.tentacles.push({
                angle: (Math.PI * 2 / 8) * i,
                length: this.radius * 1.5,
                wave: Math.random() * Math.PI * 2
            });
        }
    }
    
    update(playerX, playerY, game) {
        // Slow movement toward player
        const angle = Math.atan2(playerY - this.y, playerX - this.x);
        this.x += Math.cos(angle) * this.speed;
        this.y += Math.sin(angle) * this.speed;
        
        this.shadowPulse += 0.03;
        
        // Update tentacles
        this.tentacles.forEach(tentacle => {
            tentacle.wave += 0.1;
            tentacle.length = this.radius * (1.5 + Math.sin(tentacle.wave) * 0.3);
        });
        
        // Spawn shadow crawlers periodically
        this.swarmSpawnTimer++;
        if (this.swarmSpawnTimer >= this.swarmSpawnCooldown && game) {
            this.spawnSwarm(game);
            this.swarmSpawnTimer = 0;
        }
    }
    
    spawnSwarm(game) {
        // Spawn 3-4 shadow crawlers around the giant
        const spawnCount = 3 + Math.floor(Math.random() * 2);
        for (let i = 0; i < spawnCount; i++) {
            const spawnAngle = (Math.PI * 2 / spawnCount) * i;
            const spawnDistance = this.radius * 2;
            const spawnX = this.x + Math.cos(spawnAngle) * spawnDistance;
            const spawnY = this.y + Math.sin(spawnAngle) * spawnDistance;
            
            // Keep spawns within bounds
            const clampedX = Math.max(20, Math.min(game.width - 20, spawnX));
            const clampedY = Math.max(20, Math.min(game.height - 20, spawnY));
            
            game.enemies.push(new ShadowCrawler(clampedX, clampedY, this.wave));
        }
        
        // Add screen shake when spawning swarm
        game.addScreenShake(8, 20);
    }
    
    takeDamage() {
        this.health--;
        return this.health <= 0;
    }
    
    render(ctx) {
        const pulseIntensity = 0.3 + Math.sin(this.shadowPulse) * 0.2;
        const shadowRadius = this.radius * (3 + pulseIntensity * 0.8);
        
        // Massive shadow aura
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, shadowRadius);
        gradient.addColorStop(0, `rgba(46, 10, 51, ${0.9 + pulseIntensity * 0.1})`);
        gradient.addColorStop(0.5, `rgba(46, 10, 51, ${0.5 + pulseIntensity * 0.2})`);
        gradient.addColorStop(1, 'rgba(46, 10, 51, 0)');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, shadowRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw writhing tentacles
        ctx.strokeStyle = this.color;
        ctx.lineWidth = 4;
        ctx.lineCap = 'round';
        
        this.tentacles.forEach(tentacle => {
            const startX = this.x + Math.cos(tentacle.angle) * this.radius;
            const startY = this.y + Math.sin(tentacle.angle) * this.radius;
            const endX = startX + Math.cos(tentacle.angle) * tentacle.length;
            const endY = startY + Math.sin(tentacle.angle) * tentacle.length;
            
            ctx.beginPath();
            ctx.moveTo(startX, startY);
            ctx.quadraticCurveTo(
                startX + Math.cos(tentacle.angle + Math.PI/2) * Math.sin(tentacle.wave) * 20,
                startY + Math.sin(tentacle.angle + Math.PI/2) * Math.sin(tentacle.wave) * 20,
                endX, endY
            );
            ctx.stroke();
        });
        
        // Draw giant body with health-based intensity
        const healthRatio = this.health / this.maxHealth;
        const bodyAlpha = 0.8 + healthRatio * 0.2;
        
        ctx.fillStyle = `rgba(46, 10, 51, ${bodyAlpha})`;
        ctx.strokeStyle = '#1A051F';
        ctx.lineWidth = 4;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        ctx.stroke();
        
        // Draw massive glowing eyes
        const eyeSize = Math.max(6, this.radius / 4);
        ctx.fillStyle = '#FF0066';
        ctx.shadowBlur = 10;
        ctx.shadowColor = '#FF0066';
        ctx.beginPath();
        ctx.arc(this.x - this.radius * 0.3, this.y - this.radius * 0.3, eyeSize, 0, Math.PI * 2);
        ctx.arc(this.x + this.radius * 0.3, this.y - this.radius * 0.3, eyeSize, 0, Math.PI * 2);
        ctx.fill();
        ctx.shadowBlur = 0;
        
        // Draw health bar above boss
        const barWidth = this.radius * 2;
        const barHeight = 8;
        const barX = this.x - barWidth / 2;
        const barY = this.y - this.radius - 20;
        
        // Background
        ctx.fillStyle = 'rgba(0, 0, 0, 0.7)';
        ctx.fillRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4);
        
        // Health bar
        ctx.fillStyle = '#FF0000';
        ctx.fillRect(barX, barY, barWidth * (this.health / this.maxHealth), barHeight);
        
        // Border
        ctx.strokeStyle = '#FFD700';
        ctx.lineWidth = 2;
        ctx.strokeRect(barX - 2, barY - 2, barWidth + 4, barHeight + 4);
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
        this.speed = 20; // Doubled from 10 to 20
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
            case 'fire':
                this.vx = (Math.random() - 0.5) * 5;
                this.vy = (Math.random() - 0.5) * 5 - 1; // Slight upward bias
                this.color = '#FF6500';
                this.life = 25;
                this.maxLife = 25;
                break;
            case 'spirit':
                this.vx = (Math.random() - 0.5) * 4;
                this.vy = (Math.random() - 0.5) * 4;
                this.color = '#9932CC';
                this.life = 35;
                this.maxLife = 35;
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
        return new (this.getWeaponClass())();
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
    
    attack(player, targetX, targetY, projectiles, particles, game) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(25)) return; // Higher energy cost for AOE
        
        this.attackCooldown = this.cooldownTime;
        
        // Add powerful screen shake for thunder hammer
        if (game && game.addScreenShake) {
            game.addScreenShake(12, 25);
        }
        
        // Play thunder hammer sound
        if (game && game.soundManager) {
            game.soundManager.playSound('thunder_hammer');
        }
        
        // Create massive lightning AOE effect around player
        for (let i = 0; i < 16; i++) {
            const angle = (Math.PI * 2 / 16) * i;
            const distance = this.range * (0.8 + Math.random() * 0.4);
            particles.push(new Particle(
                player.x + Math.cos(angle) * distance,
                player.y + Math.sin(angle) * distance,
                'lightning'
            ));
        }
        
        // Create central explosion effect
        for (let i = 0; i < 12; i++) {
            particles.push(new Particle(
                player.x + (Math.random() - 0.5) * 40,
                player.y + (Math.random() - 0.5) * 40,
                'explosion'
            ));
        }
        
        // Create AOE damage around player
        if (game && game.enemies) {
            const enemies = game.enemies;
            
            for (let i = enemies.length - 1; i >= 0; i--) {
                const enemy = enemies[i];
                const dx = enemy.x - player.x;
                const dy = enemy.y - player.y;
                const distance = Math.sqrt(dx * dx + dy * dy);
                
                if (distance <= this.range) {
                    // Create lightning strike effect to enemy
                    for (let j = 0; j < 3; j++) {
                        particles.push(new Particle(
                            enemy.x + (Math.random() - 0.5) * 20,
                            enemy.y + (Math.random() - 0.5) * 20,
                            'lightning'
                        ));
                    }
                    
                    // Create explosion particles
                    for (let j = 0; j < 8; j++) {
                        particles.push(new Particle(enemy.x, enemy.y, 'explosion'));
                    }
                    
                    // Create sparkle effects
                    if (game.createSparkleEffect) {
                        game.createSparkleEffect(enemy.x, enemy.y);
                    }
                    
                    // Remove enemy and award points
                    enemies.splice(i, 1);
                    game.score += 15 * game.wave; // Bonus points for AOE weapon
                }
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
    
    attack(player, targetX, targetY, projectiles, particles, game) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(15)) return; // Higher energy cost
        
        this.attackCooldown = this.cooldownTime;
        
        // Add moderate screen shake for phoenix bow
        if (game && game.addScreenShake) {
            game.addScreenShake(6, 15);
        }
        
        // Play phoenix bow sound
        if (game && game.soundManager) {
            game.soundManager.playSound('phoenix_bow');
        }
        
        const centerAngle = Math.atan2(targetY - player.y, targetX - player.x);
        const spread = Math.PI / 6; // 30 degree spread
        
        // Fire 3 arrows in a spread with enhanced fire trails
        for (let i = 0; i < this.arrowCount; i++) {
            const angle = centerAngle + (i - 1) * spread / 2;
            const arrow = new PhoenixArrow(player.x, player.y, angle);
            arrow.hasFireTrails = true; // Enable fire trails
            projectiles.push(arrow);
        }
        
        // Create phoenix fire burst effect at launch
        for (let i = 0; i < 12; i++) {
            const angle = Math.random() * Math.PI * 2;
            const distance = Math.random() * 30;
            particles.push(new Particle(
                player.x + Math.cos(angle) * distance,
                player.y + Math.sin(angle) * distance,
                'fire'
            ));
        }
        
        // Create muzzle flash effect
        for (let i = 0; i < 8; i++) {
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
    
    attack(player, targetX, targetY, projectiles, particles, game) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(20)) return;
        
        this.attackCooldown = this.cooldownTime;
        this.isActive = true;
        this.shieldTimer = this.shieldDuration;
        
        // Add light screen shake for shield activation
        if (game && game.addScreenShake) {
            game.addScreenShake(4, 10);
        }
        
        // Play shield sound
        if (game && game.soundManager) {
            game.soundManager.playSound('shield_activate');
        }
        
        // Create shield activation burst
        for (let i = 0; i < 16; i++) {
            const angle = (Math.PI * 2 / 16) * i;
            particles.push(new Particle(
                player.x + Math.cos(angle) * 35,
                player.y + Math.sin(angle) * 35,
                'shield'
            ));
        }
        
        // Create inner light burst
        for (let i = 0; i < 8; i++) {
            particles.push(new Particle(
                player.x + (Math.random() - 0.5) * 20,
                player.y + (Math.random() - 0.5) * 20,
                'light'
            ));
        }
    }
    
    render(ctx, player) {
        if (!this.isActive) return;
        
        // Draw visible bubble shield around player
        const alpha = this.shieldTimer / this.shieldDuration;
        const pulseRadius = 35 + Math.sin(Date.now() * 0.005) * 8;
        const shimmer = Math.sin(Date.now() * 0.01) * 0.3 + 0.7;
        
        ctx.save();
        
        // Draw outer shield glow
        const outerGradient = ctx.createRadialGradient(
            player.x, player.y, pulseRadius * 0.8,
            player.x, player.y, pulseRadius * 1.2
        );
        outerGradient.addColorStop(0, `rgba(0, 191, 255, ${alpha * 0.1})`);
        outerGradient.addColorStop(0.7, `rgba(135, 206, 235, ${alpha * 0.3})`);
        outerGradient.addColorStop(1, `rgba(255, 255, 255, ${alpha * 0.5})`);
        
        ctx.fillStyle = outerGradient;
        ctx.beginPath();
        ctx.arc(player.x, player.y, pulseRadius, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw main shield bubble
        const shieldGradient = ctx.createRadialGradient(
            player.x - pulseRadius * 0.3, player.y - pulseRadius * 0.3, 0,
            player.x, player.y, pulseRadius
        );
        shieldGradient.addColorStop(0, `rgba(255, 255, 255, ${alpha * 0.4 * shimmer})`);
        shieldGradient.addColorStop(0.3, `rgba(135, 206, 235, ${alpha * 0.3})`);
        shieldGradient.addColorStop(0.8, `rgba(0, 191, 255, ${alpha * 0.2})`);
        shieldGradient.addColorStop(1, `rgba(0, 100, 200, ${alpha * 0.1})`);
        
        ctx.fillStyle = shieldGradient;
        ctx.beginPath();
        ctx.arc(player.x, player.y, pulseRadius * 0.95, 0, Math.PI * 2);
        ctx.fill();
        
        // Draw shield border with energy crackling effect
        ctx.globalAlpha = alpha * 0.8;
        ctx.strokeStyle = `rgba(255, 255, 255, ${shimmer})`;
        ctx.lineWidth = 3;
        ctx.setLineDash([5, 3]);
        ctx.beginPath();
        ctx.arc(player.x, player.y, pulseRadius, 0, Math.PI * 2);
        ctx.stroke();
        ctx.setLineDash([]);
        
        // Draw inner energy ring
        ctx.globalAlpha = alpha * 0.6;
        ctx.strokeStyle = '#00BFFF';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(player.x, player.y, pulseRadius * 0.7, 0, Math.PI * 2);
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
    
    attack(player, targetX, targetY, projectiles, particles, game) {
        if (!this.canAttack()) return;
        if (!player.useLightEnergy(3)) return; // Low energy per shot
        
        this.attackCooldown = this.cooldownTime;
        this.burstCount++;
        
        // Add subtle screen shake for rapid fire
        if (game && game.addScreenShake) {
            game.addScreenShake(2, 5);
        }
        
        // Play spirit cannon sound
        if (game && game.soundManager) {
            game.soundManager.playSound('spirit_cannon');
        }
        
        const angle = Math.atan2(targetY - player.y, targetX - player.x);
        projectiles.push(new SpiritBlast(player.x, player.y, angle));
        
        if (this.burstCount >= this.maxBurst) {
            this.burstCount = 0;
            this.attackCooldown = this.burstCooldown;
        }
        
        // Create enhanced muzzle flash in direction of shot
        const muzzleDistance = 20;
        const muzzleX = player.x + Math.cos(angle) * muzzleDistance;
        const muzzleY = player.y + Math.sin(angle) * muzzleDistance;
        
        // Main muzzle flash
        for (let i = 0; i < 5; i++) {
            const flashAngle = angle + (Math.random() - 0.5) * 0.8;
            const flashDistance = Math.random() * 15 + 10;
            particles.push(new Particle(
                muzzleX + Math.cos(flashAngle) * flashDistance,
                muzzleY + Math.sin(flashAngle) * flashDistance,
                'spirit'
            ));
        }
        
        // Barrel smoke effect
        for (let i = 0; i < 3; i++) {
            particles.push(new Particle(
                muzzleX + (Math.random() - 0.5) * 8,
                muzzleY + (Math.random() - 0.5) * 8,
                'light'
            ));
        }
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
        this.angle = angle;
        this.hasFireTrails = false;
        this.trailParticles = [];
        this.trailTimer = 0;
    }
    
    update() {
        super.update();
        
        // Create fire trail particles if enabled
        if (this.hasFireTrails) {
            this.trailTimer++;
            if (this.trailTimer % 3 === 0) { // Create trail every 3 frames
                this.trailParticles.push({
                    x: this.x + (Math.random() - 0.5) * 8,
                    y: this.y + (Math.random() - 0.5) * 8,
                    life: 20,
                    maxLife: 20,
                    size: Math.random() * 3 + 2
                });
                
                // Limit trail length
                if (this.trailParticles.length > 8) {
                    this.trailParticles.shift();
                }
            }
            
            // Update trail particles
            for (let i = this.trailParticles.length - 1; i >= 0; i--) {
                this.trailParticles[i].life--;
                if (this.trailParticles[i].life <= 0) {
                    this.trailParticles.splice(i, 1);
                }
            }
        }
    }
    
    render(ctx) {
        // Draw fire trail first (behind arrow)
        if (this.hasFireTrails) {
            this.trailParticles.forEach(particle => {
                const alpha = particle.life / particle.maxLife;
                const colors = ['#FF6500', '#FF4500', '#FF8C00', '#FFD700'];
                const colorIndex = Math.floor((1 - alpha) * colors.length);
                
                ctx.fillStyle = `rgba(255, ${69 + colorIndex * 30}, 0, ${alpha * 0.8})`;
                ctx.beginPath();
                ctx.arc(particle.x, particle.y, particle.size * alpha, 0, Math.PI * 2);
                ctx.fill();
            });
        }
        
        // Draw phoenix arrow with enhanced fire effect
        const gradient = ctx.createRadialGradient(this.x, this.y, 0, this.x, this.y, this.radius * 2);
        gradient.addColorStop(0, '#FFFFFF');
        gradient.addColorStop(0.3, '#FFFF00');
        gradient.addColorStop(0.6, '#FF6500');
        gradient.addColorStop(1, '#FF4500');
        
        ctx.fillStyle = gradient;
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.radius, 0, Math.PI * 2);
        ctx.fill();
        
        // Add outer glow
        ctx.shadowBlur = 10;
        ctx.shadowColor = '#FF4500';
        ctx.fill();
        ctx.shadowBlur = 0;
        
        // Draw arrowhead pointing in direction of movement
        ctx.fillStyle = '#FFD700';
        ctx.save();
        ctx.translate(this.x, this.y);
        ctx.rotate(this.angle);
        ctx.beginPath();
        ctx.moveTo(this.radius, 0);
        ctx.lineTo(-this.radius * 0.5, -this.radius * 0.5);
        ctx.lineTo(-this.radius * 0.5, this.radius * 0.5);
        ctx.closePath();
        ctx.fill();
        ctx.restore();
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

// High Score Manager Class
class HighScoreManager {
    constructor() {
        this.storageKey = 'phoenixLightWarrior_highScores';
        this.maxScores = 5;
        this.scores = this.loadScores();
    }
    
    loadScores() {
        try {
            const stored = localStorage.getItem(this.storageKey);
            if (stored) {
                const parsed = JSON.parse(stored);
                // Ensure we have valid score objects
                return parsed.filter(score => 
                    score && 
                    typeof score.score === 'number' && 
                    typeof score.wave === 'number' && 
                    typeof score.playerName === 'string'
                );
            }
        } catch (error) {
            console.warn('Error loading high scores:', error);
        }
        return [];
    }
    
    saveScores() {
        try {
            localStorage.setItem(this.storageKey, JSON.stringify(this.scores));
        } catch (error) {
            console.warn('Error saving high scores:', error);
        }
    }
    
    isNewHighScore(score, wave) {
        // Check if this score would make it into the top 5
        if (this.scores.length < this.maxScores) {
            return true;
        }
        
        // Check if score is higher than lowest high score
        const lowestScore = this.scores[this.scores.length - 1];
        return score > lowestScore.score || (score === lowestScore.score && wave > lowestScore.wave);
    }
    
    addScore(playerName, score, wave) {
        const newScore = {
            playerName: playerName || 'Anonymous',
            score: score,
            wave: wave,
            date: new Date().toLocaleDateString(),
            timestamp: Date.now()
        };
        
        this.scores.push(newScore);
        
        // Sort by score descending, then by wave descending
        this.scores.sort((a, b) => {
            if (b.score !== a.score) {
                return b.score - a.score;
            }
            return b.wave - a.wave;
        });
        
        // Keep only top scores
        this.scores = this.scores.slice(0, this.maxScores);
        
        this.saveScores();
        
        // Return the rank (1-based)
        return this.scores.findIndex(s => s.timestamp === newScore.timestamp) + 1;
    }
    
    getTopScores() {
        return [...this.scores]; // Return copy to prevent external modifications
    }
    
    getHighestScore() {
        return this.scores.length > 0 ? this.scores[0].score : 0;
    }
    
    getHighestWave() {
        return this.scores.length > 0 ? Math.max(...this.scores.map(s => s.wave)) : 1;
    }
    
    clearScores() {
        this.scores = [];
        this.saveScores();
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
        
        this.createSoundEffect('weapon_switch', {
            frequency: 800,
            type: 'square',
            duration: 0.15,
            volume: 0.3,
            decay: 0.05,
            sweep: { start: 800, end: 1200 }
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
        
        this.createSoundEffect('high_score_celebration', {
            frequency: 800,
            type: 'triangle',
            duration: 1.5,
            volume: 0.6,
            decay: 0.4,
            chord: [800, 1000, 1200, 1600]
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