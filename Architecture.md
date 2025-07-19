# Phoenix: Light Warrior - Game Architecture

## Overview
Phoenix: Light Warrior is a comprehensive wave-based survival game built with modern web technologies. The player controls a Light Warrior defending sacred ground against waves of Shadow Demons using light magic and special weapons. Features include high score persistence, mobile support, audio system, and analytics integration.

## Project Structure

```
light-warrior/
├── index.html          # Main game HTML with complete UI system
├── style.css           # Comprehensive styling with responsive design
├── game.js             # Complete game engine with all systems
├── favicon.ico         # Browser tab icon
├── README.md           # Comprehensive game documentation
├── Architecture.md     # This technical architecture documentation
└── .claude/
    └── settings.local.json  # Development configuration
```

## Core Architecture

### Game Engine (`Game` Class)

The main game engine manages the overall game state and coordinates all systems:

**Properties:**
- `gameState`: Controls game flow ('menu', 'playing', 'paused', 'gameOver')
- `waveState`: Manages wave progression ('spawning', 'active', 'complete', 'intermission')
- `player`: Player character instance
- `enemies[]`: Array of active enemy instances
- `projectiles[]`: Array of active projectile instances
- `particles[]`: Array of visual effect particles
- `weaponShrines[]`: Array of hidden weapon pickup locations
- `activeWeapon`: Currently equipped special weapon
- `soundManager`: Audio system manager
- `highScoreManager`: Persistent scoring system

**Key Systems:**
- **Wave Management**: Dynamic wave system with automatic progression
- **High Score System**: Top 5 leaderboard with localStorage persistence
- **Special Weapons**: Hidden shrines with unique combat abilities
- **Mobile Controls**: Virtual joystick and touch button system
- **Sound System**: Web Audio API with procedural generation
- **Analytics**: Google Analytics 4 event tracking
- **Collision Detection**: Optimized circle-based collision system
- **Rendering Pipeline**: Multi-layered rendering with effects
- **Input Handling**: Cross-platform input (keyboard, mouse, touch)

### Player System (`Player` Class)

**Core Features:**
- **Physics-Based Movement**: Velocity, acceleration, and friction system
- **Health & Energy**: 100 HP health, 100 Light Energy with regeneration
- **Attack System**: Light Orb projectiles that cost energy
- **Visual Effects**: Pulsing light aura, health-based color changes
- **Collision**: Circular collision boundary with proper physics response

**Movement System:**
```javascript
// Smooth velocity-based movement
this.vx += inputX * this.acceleration;
this.vy += inputY * this.acceleration;
// Apply friction and speed limiting
// Update position based on velocity
```

### Enemy System (`ShadowDemon` Class)

**Behavior:**
- **AI Movement**: Move toward player with slight wobble variation
- **Progressive Scaling**: Size and speed increase with wave number
- **Visual Effects**: Pulsing shadow auras, glowing red eyes
- **Wave-Based Features**: Shadow tendrils appear on Wave 3+

**Spawn System:**
- Enemies spawn from random screen edges (top/right/bottom/left)
- Timed spawning with decreasing delays between enemies
- Wave-based enemy count: 8 + (wave-1) * 2

### Projectile System (`LightOrb` Class)

**Features:**
- **Enhanced Visuals**: Multi-layered glow effects with particle trails
- **Physics**: Straight-line movement with lifetime management
- **Trail Effects**: Fading particle trail behind each orb
- **Impact**: Destroys enemies on collision with sparkle effects

### Particle System (`Particle` Class)

**Particle Types:**
- **Light**: Golden burst effects for attacks
- **Explosion**: Orange/red particles for enemy destruction
- **Sparkle**: 4-pointed star effects when enemies are defeated
- **Healing**: Green cross particles for health restoration
- **Damage**: Red particles when player takes damage

### Wave Management System

**Wave States:**
1. **Spawning**: Gradually spawn enemies from screen edges
2. **Active**: Combat phase until all enemies defeated
3. **Complete**: Victory moment with "Wave Complete!" message
4. **Intermission**: 3-second countdown with health restoration

**Progression:**
- Enemy count increases by +4 per wave (Wave 1: 8, Wave 2: 12, etc.)
- Enemy spawn rate increases (faster spawning)
- Enemy stats scale with wave number
- Player fully heals between waves
- Weapon shrines reset for each new wave

## Data Flow

```
Game Loop (60 FPS)
├── Input Processing (keyboard/mouse)
├── Update Phase
│   ├── Player Update (movement, physics)
│   ├── Enemy Updates (AI, movement)
│   ├── Projectile Updates (movement, lifetime)
│   ├── Particle Updates (effects, lifetime)
│   ├── Wave System Update (spawning, state management)
│   └── Collision Detection (all objects)
└── Render Phase
    ├── Background (gradient, sacred circle)
    ├── Particles (background layer)
    ├── Game Objects (player, enemies, projectiles)
    ├── UI Elements (health, energy, wave, score)
    └── Overlays (pause, intermission, messages)
```

## Key Algorithms

### Collision Detection
```javascript
checkCircleCollision(obj1, obj2) {
    const dx = obj1.x - obj2.x;
    const dy = obj1.y - obj2.y;
    const distance = Math.sqrt(dx * dx + dy * dy);
    return distance < (obj1.radius + obj2.radius);
}
```

### Movement Physics
```javascript
// Apply input acceleration
this.vx += inputX * this.acceleration;
this.vy += inputY * this.acceleration;

// Apply friction
this.vx *= this.friction;
this.vy *= this.friction;

// Limit maximum speed
const speed = Math.sqrt(this.vx * this.vx + this.vy * this.vy);
if (speed > this.maxSpeed) {
    this.vx = (this.vx / speed) * this.maxSpeed;
    this.vy = (this.vy / speed) * this.maxSpeed;
}
```

### Enemy Spawning
```javascript
spawnEnemyFromEdge() {
    const side = Math.floor(Math.random() * 4);
    // Calculate position outside screen boundary
    // Create ShadowDemon with current wave stats
}
```

## Advanced Systems Architecture

### High Score System (`HighScoreManager` Class)

**Core Features:**
- **localStorage Persistence**: Saves top 5 scores across browser sessions
- **Data Validation**: Ensures score integrity with type checking
- **Ranking System**: Automatic sorting by score then wave progression
- **Player Names**: Supports custom player names with validation

**Data Structure:**
```javascript
{
    playerName: 'Player1',
    score: 12450,
    wave: 8,
    date: '12/25/2023',
    timestamp: 1703524800000
}
```

**Key Methods:**
- `isNewHighScore(score, wave)`: Determines if score qualifies for leaderboard
- `addScore(name, score, wave)`: Adds and ranks new high score
- `getTopScores()`: Returns sorted array of top 5 scores
- `loadScores()` / `saveScores()`: localStorage persistence with error handling

### Sound System (`SoundManager` Class)

**Architecture:**
- **Web Audio API**: Procedural sound generation using oscillators
- **No External Files**: All audio synthesized in browser
- **Graceful Fallback**: Continues operation if Web Audio unavailable
- **Performance Optimized**: Minimal memory footprint

**Sound Types:**
- **Combat Sounds**: Shooting, enemy defeat, weapon usage
- **UI Sounds**: High score celebration, wave completion
- **Background Music**: Heroic melody loop with triangle waves
- **Environmental**: Player hurt, weapon pickup effects

**Technical Implementation:**
```javascript
playSingleTone(audioContext, frequency, config) {
    const oscillator = audioContext.createOscillator();
    const gainNode = audioContext.createGain();
    
    // Configure waveform, frequency, volume envelope
    // Support for chord progressions and modulation
    // Automatic cleanup after duration
}
```

### Special Weapons System

**Weapon Classes Hierarchy:**
```
Weapon (Base Class)
├── ThunderHammer (AOE Lightning)
├── PhoenixBow (Multi-shot Arrows)
├── ShieldOfLight (Protective Barrier)
└── SpiritCannon (Rapid-fire Blasts)
```

**Weapon Shrine System:**
- **Strategic Placement**: Located at arena corners
- **Visual Indicators**: Color-coded glowing shrines
- **Pickup Mechanics**: Collision-based activation
- **Reset Behavior**: Reactivate each wave

**Combat Integration:**
- **Energy Consumption**: Varied costs per weapon type
- **Cooldown Systems**: Balanced attack rates
- **Visual Effects**: Unique particle systems per weapon
- **Damage Scaling**: Bonus scoring for weapon usage

### Mobile Control System

**Virtual Joystick:**
- **Touch Area Detection**: 120px circular boundary
- **Analog Input**: Proportional movement based on touch distance
- **Visual Feedback**: Knob position updates in real-time
- **Dead Zone**: Prevents accidental activation

**Touch Button System:**
- **Fire Button**: Primary attack with visual feedback
- **Weapon Button**: Special ability activation
- **Press States**: Visual feedback with CSS classes
- **Gesture Prevention**: Prevents page scrolling during gameplay

**Responsive Design:**
```javascript
// Auto-detection and layout adjustment
this.isMobile = this.detectMobile();
if (this.isMobile) {
    this.setupMobileControls();
    // Show mobile UI elements
    // Hide desktop-specific instructions
}
```

### Analytics Integration

**Google Analytics 4 Events:**
- **Game Play**: Start game, restart game
- **Game Over**: Completion with score value
- **Level Up**: Wave progression tracking
- **User Engagement**: Session duration and interaction

**Event Structure:**
```javascript
gtag('event', 'game_play', {
    'game_name': 'Phoenix Light Warrior',
    'event_category': 'game_interaction',
    'event_label': 'start_game'
});
```

## Performance Considerations

### Object Management
- **Object Pooling**: Particles and projectiles are removed when off-screen
- **Efficient Updates**: Only update active objects
- **Collision Optimization**: Early exit on collision detection
- **Memory Cleanup**: Automatic cleanup of expired sound contexts

### Rendering Optimization
- **Layered Rendering**: Background → Shrines → Particles → Objects → UI
- **Canvas State Management**: Proper save/restore for effects
- **Gradient Caching**: Reuse gradient calculations where possible
- **Mobile Optimization**: Scaled rendering for smaller screens

### Memory Management
- **Array Cleanup**: Remove dead objects immediately
- **Event Listeners**: Properly managed without memory leaks
- **localStorage Optimization**: Minimal data footprint for high scores
- **Audio Context Cleanup**: Automatic disposal of completed audio nodes
- **Animation Frames**: Single requestAnimationFrame loop

## Extensibility Points

### Adding New Enemy Types
1. Create new class extending `ShadowDemon`
2. Override `update()` and `render()` methods
3. Add to wave spawning logic with conditional wave triggers
4. Integrate with sound system for unique audio effects

### Adding New Weapons
1. Create new class extending `Weapon` base class
2. Implement `attack()` method with unique mechanics
3. Add to `WeaponShrine.getWeaponClass()` switch statement
4. Create custom projectile class if needed
5. Add sound effects to `SoundManager`

### Expanding High Score Features
1. Add new tracking metrics (time survived, accuracy, etc.)
2. Implement achievement system with localStorage
3. Add social sharing integration
4. Create detailed statistics dashboard

### Adding More Particle Effects
1. Add new particle types to `Particle` class constructor
2. Implement rendering logic in `render()` method
3. Create helper methods in `Game` class for specific effects
4. Integrate with weapon systems for unique visuals

## Dependencies

### Core Technologies
- **HTML5 Canvas**: 2D rendering engine with responsive scaling
- **JavaScript ES6+**: Modern class-based architecture
- **CSS3 Grid & Flexbox**: Advanced layout systems
- **Web Audio API**: Procedural sound generation
- **localStorage API**: Persistent data storage
- **Touch Events API**: Mobile gesture handling
- **Google Analytics 4**: User behavior tracking

### Browser API Requirements
- **HTML5 Canvas 2D Context**: Core rendering (Required)
- **localStorage**: High score persistence (Required)
- **Web Audio API**: Sound system (Optional - graceful fallback)
- **Touch Events**: Mobile controls (Required for mobile)
- **RequestAnimationFrame**: Smooth animation (Required)

### Browser Compatibility
- **Desktop**: Chrome 50+, Firefox 45+, Safari 10+, Edge 79+
- **Mobile**: iOS Safari 10+, Chrome Mobile 50+, Samsung Internet
- **Fallback Support**: Game continues without audio if Web Audio unavailable

## Configuration Values

### Gameplay Constants
```javascript
// Player System
MAX_SPEED = 4
ACCELERATION = 0.3
FRICTION = 0.85
MAX_HEALTH = 100
MAX_LIGHT_ENERGY = 100
ENERGY_REGEN_RATE = 0.5

// Wave System  
BASE_ENEMY_COUNT = 8
ENEMIES_PER_WAVE_INCREASE = 4
INTERMISSION_DURATION = 180 // 3 seconds at 60fps
MIN_ENEMY_SPAWN_DELAY = 15  // Minimum frames between spawns

// Combat System
LIGHT_ORB_ENERGY_COST = 10
ATTACK_COOLDOWN = 12 // frames
PROJECTILE_SPEED = 10
PROJECTILE_LIFETIME = 120 // frames

// High Score System
MAX_STORED_SCORES = 5
MAX_PLAYER_NAME_LENGTH = 20
HIGH_SCORE_STORAGE_KEY = 'phoenixLightWarrior_highScores'

// Special Weapons
THUNDER_HAMMER_COOLDOWN = 45
PHOENIX_BOW_COOLDOWN = 20  
SHIELD_DURATION = 120
SPIRIT_CANNON_BURST_SIZE = 8

// Mobile Controls
JOYSTICK_SIZE = 120
JOYSTICK_DEAD_ZONE = 20
MOBILE_BUTTON_SIZE = 70
```

## Future Architecture Improvements

### Potential Future Enhancements
1. **Component System**: Break down classes into reusable components
2. **State Management**: Centralized immutable state updates  
3. **Asset Loading**: Sprite and texture management system
4. **Cloud Saves**: Server-based score synchronization
5. **Multiplayer Foundation**: WebSocket-based cooperative play
6. **Progressive Web App**: Service worker for offline capability

### Code Organization Improvements
- **Module System**: Split classes into ES6 modules
- **Config File**: Centralize all game constants in JSON
- **Utils Module**: Shared utility functions and helpers
- **Asset Manager**: Centralized resource loading and caching
- **Event System**: Pub/sub pattern for loose coupling
- **Physics Engine**: Dedicated physics system for complex interactions

## Testing Strategy

### Current Testing Approach
- **Manual Browser Testing**: Cross-platform device testing
- **Performance Monitoring**: Browser dev tools and FPS analysis
- **localStorage Testing**: High score persistence validation
- **Mobile Device Testing**: Touch controls and responsive design
- **Audio System Testing**: Web Audio API compatibility checks

### Planned Testing Improvements
- **Unit Tests**: Core game logic with Jest or similar framework
- **Integration Tests**: System interaction validation
- **Performance Benchmarks**: Automated FPS and memory usage tests
- **Cross-browser CI**: Automated compatibility testing
- **User Acceptance Testing**: Player feedback and usability studies

## Current System Status

### ✅ Implemented Features (v2.0)
- Complete wave-based survival gameplay
- High score system with localStorage persistence
- Mobile-responsive design with virtual controls
- Comprehensive Web Audio API sound system  
- Special weapons system with 4 unique types
- Google Analytics 4 event tracking integration
- Advanced particle effects and visual polish

### 📊 Technical Metrics
- **Lines of Code**: ~2000+ JavaScript, 800+ CSS
- **Classes**: 15+ game logic classes
- **Performance**: 60 FPS target with mobile optimization
- **Storage**: <1KB localStorage footprint
- **Compatibility**: 95%+ modern browser support

---

*Last Updated: December 2024*  
*Game Version: 2.0.0 (High Score System)*  
*Architecture Version: 2.0*