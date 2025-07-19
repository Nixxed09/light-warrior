# Phoenix: Light Warrior - Game Architecture

## Overview
Phoenix: Light Warrior is a wave-based survival game built with HTML5 Canvas and JavaScript. The player controls a Light Warrior defending sacred ground against waves of Shadow Demons using light magic projectiles.

## Project Structure

```
light-warrior/
├── index.html          # Main game HTML file with UI elements
├── style.css           # Visual styling and effects
├── game.js             # Core game engine and all classes
├── README.md           # Game documentation and usage instructions
├── Architecture.md     # This architecture documentation
└── assets/
    ├── images/         # Future sprite and image assets
    └── sounds/         # Future audio assets
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

**Key Systems:**
- **Wave Management**: Progressive difficulty with timed enemy spawning
- **Collision Detection**: Circle-based collision system for all game objects
- **Rendering Pipeline**: Layered rendering with background → particles → objects
- **Input Handling**: Keyboard (WASD/arrows) and mouse controls
- **UI Management**: Health, energy, wave, and score displays

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
- Enemy count increases by +2 per wave
- Enemy spawn rate increases (faster spawning)
- Enemy stats scale with wave number
- Player fully heals between waves

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

## Performance Considerations

### Object Management
- **Object Pooling**: Particles and projectiles are removed when off-screen
- **Efficient Updates**: Only update active objects
- **Collision Optimization**: Early exit on collision detection

### Rendering Optimization
- **Layered Rendering**: Background → Effects → Objects → UI
- **Canvas State Management**: Proper save/restore for effects
- **Gradient Caching**: Reuse gradient calculations where possible

### Memory Management
- **Array Cleanup**: Remove dead objects immediately
- **Event Listeners**: Properly managed without memory leaks
- **Animation Frames**: Single requestAnimationFrame loop

## Extensibility Points

### Adding New Enemy Types
1. Create new class extending `ShadowDemon`
2. Override `update()` and `render()` methods
3. Add to wave spawning logic

### Adding Power-ups
1. Create `PowerUp` class similar to projectiles
2. Add collision detection with player
3. Implement effect application system

### Adding Sound System
1. Create `SoundManager` class
2. Load audio files into `assets/sounds/`
3. Integrate with game events (attacks, explosions, etc.)

### Adding More Particle Effects
1. Add new particle types to `Particle` class constructor
2. Implement rendering logic in `render()` method
3. Create helper methods in `Game` class for specific effects

## Dependencies

### Core Technologies
- **HTML5 Canvas**: 2D rendering engine
- **JavaScript ES6+**: Game logic and classes
- **CSS3**: UI styling and animations

### Browser Compatibility
- Chrome 50+, Firefox 45+, Safari 10+, Edge 79+
- Requires HTML5 Canvas and ES6 class support

## Configuration Values

### Gameplay Constants
```javascript
// Player
MAX_SPEED = 4
ACCELERATION = 0.3
FRICTION = 0.85
MAX_HEALTH = 100
MAX_LIGHT_ENERGY = 100

// Wave System
BASE_ENEMY_COUNT = 8
ENEMIES_PER_WAVE = 2
INTERMISSION_DURATION = 180 // 3 seconds at 60fps

// Combat
LIGHT_ORB_ENERGY_COST = 10
ATTACK_COOLDOWN = 12 // frames
PROJECTILE_SPEED = 10
```

## Future Architecture Improvements

### Planned Enhancements
1. **Component System**: Break down classes into reusable components
2. **State Management**: Centralized game state with immutable updates  
3. **Asset Loading**: Proper image and audio asset management
4. **Save System**: LocalStorage integration for high scores
5. **Multiplayer Foundation**: Network architecture preparation

### Code Organization
- **Separate Files**: Split classes into individual modules
- **Config File**: Centralize all game constants
- **Utils Module**: Shared utility functions
- **Asset Manager**: Centralized resource loading

## Testing Strategy

### Current Testing
- Manual browser testing across different devices
- Performance monitoring via browser dev tools

### Planned Testing
- Unit tests for core game logic
- Integration tests for game systems
- Performance benchmarks
- Cross-browser compatibility testing

---

*Last Updated: [Current Date]*  
*Game Version: 1.0.0*  
*Architecture Version: 1.0*