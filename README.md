# Phoenix: Light Warrior

A wave-based survival game where you play as a Warrior of Light defending sacred ground against endless waves of shadow demons in a mystical arena.

## Game Description

In **Phoenix: Light Warrior**, you are the guardian of sacred ground, wielding the power of light magic to fend off increasingly dangerous waves of shadow demons. The game features bright, heroic visuals perfect for players of all ages, with a classic light vs. dark theme.

### Core Gameplay
- **Wave-based Survival**: Survive endless waves of shadow demons that get stronger and more numerous over time
- **Sacred Ground Defense**: Protect the mystical circle at the center of the arena
- **Light Magic Combat**: Click to cast light projectiles that destroy shadow demons
- **Progressive Difficulty**: Each wave brings more enemies with increased speed and power
- **High Score Competition**: Compete for the top 5 leaderboard positions with persistent scoring
- **Special Weapons**: Discover hidden weapon shrines with unique magical abilities

### Controls

**Desktop:**
- **WASD** or **Arrow Keys**: Move the Light Warrior
- **Mouse Click**: Cast light magic projectiles toward cursor position
- **Start Game**: Begin defending the sacred ground
- **Pause**: Pause/Resume the action
- **Sound Toggle**: Mute/unmute game audio

**Mobile:**
- **Virtual Joystick**: Touch and drag to move the Light Warrior
- **Fire Button**: Tap to cast light magic
- **Weapon Button**: Use special weapon abilities
- **Touch Controls**: Responsive design for mobile devices

### Game Features
- **High Score System**: Top 5 leaderboard with player names and persistent localStorage
- **Mobile Support**: Full mobile compatibility with touch controls and responsive design
- **Sound System**: Comprehensive audio with background music and procedural sound effects
- **Special Weapons**: Hidden weapon shrines with unique abilities:
  - Thunder Hammer (AOE lightning attacks)
  - Phoenix Bow (Multi-shot arrows)
  - Shield of Light (Protective barrier)
  - Spirit Cannon (Rapid-fire bursts)
- **Visual Effects**: Particle systems, glowing auras, and magical combat effects
- **Analytics**: Google Analytics 4 integration for gameplay tracking
- **Celebration System**: "NEW HIGH SCORE!" animations and sound effects
- Bright, colorful graphics with heroic light vs. dark theme
- Progressive wave system with increasing difficulty
- Score tracking and health management

### Visual Style
The game uses a bright, heroic art style suitable for kids featuring:
- Golden light effects and auras
- Vibrant gradient backgrounds
- Glowing magical projectiles
- Shadow demons with menacing red eyes
- Sacred golden circle marking the defended area

### Technical Implementation
- **HTML5 Canvas**: Smooth 2D rendering with responsive scaling
- **Object-Oriented JavaScript**: Clean class-based architecture
- **localStorage API**: Persistent high score data with error handling
- **Web Audio API**: Procedural sound generation and background music
- **Touch Events**: Mobile gesture handling and virtual controls
- **CSS Grid & Flexbox**: Modern responsive layout design
- **Google Analytics 4**: Event tracking and user analytics
- **Particle System**: Dynamic visual effects and animations
- **Collision Detection**: Efficient circular collision algorithms
- **State Management**: Game state system with pause/resume functionality

## File Structure
```
light-warrior/
├── index.html          # Main game HTML with UI elements
├── style.css           # Comprehensive styling and animations
├── game.js             # Complete game engine with all classes
├── favicon.ico         # Browser tab icon
├── README.md           # This comprehensive documentation
├── Architecture.md     # Technical architecture documentation
└── .claude/
    └── settings.local.json  # Development settings
```

## How to Play

### Getting Started
1. Open `index.html` in a web browser
2. Review the high scores leaderboard on the main menu
3. Click "Start Game" to begin your adventure

### Basic Gameplay
1. **Movement**: Use WASD or arrow keys (desktop) or virtual joystick (mobile)
2. **Combat**: Click or tap to cast light magic toward enemies
3. **Survival**: Destroy shadow demons before they reach you
4. **Progression**: Complete waves to face stronger enemies

### Special Features
1. **Weapon Shrines**: Find glowing shrines at the arena corners to unlock special weapons
2. **High Scores**: Try to reach the top 5 leaderboard
3. **Wave Mastery**: Each wave increases difficulty with more and stronger enemies
4. **Health Management**: Rest between waves to restore health and energy

### Scoring System
- Earn points for each enemy defeated
- Score multiplied by current wave number
- Bonus points for using special weapons
- Compete for leaderboard positions

## Game Mechanics

### Health System
- Start with 100 health points
- Lose 20 health when touched by a shadow demon
- Health warning appears when below 30 health
- Game ends when health reaches 0

### High Score System
- **Leaderboard**: Top 5 scores persistently saved using localStorage
- **Player Names**: Enter your name when achieving a high score
- **Celebration**: "NEW HIGH SCORE!" animation with special sound effects
- **Ranking**: Display your position on the leaderboard after each game
- **Data Tracking**: Stores player name, score, wave reached, and date

### Scoring Mechanics
- Earn 10 points per enemy destroyed
- Points multiplied by current wave number
- Bonus points for special weapon usage (15 points per kill)
- Score displayed in real-time with comma formatting

### Wave Progression
- **Dynamic Wave System**: Waves transition automatically when all enemies are defeated
- **Difficulty Scaling**: Each wave adds 4 more enemies (Wave 1: 8, Wave 2: 12, etc.)
- **Speed Increases**: Enemy spawn rate and movement speed increase each wave
- **Size Growth**: Enemies become larger and more menacing over time
- **Intermission**: 3-second break between waves with countdown display
- **Health Restoration**: Full health and energy restored between waves

### Combat System
- **Light Magic**: Default light orb projectiles with trailing particle effects
- **Energy System**: Light energy consumption with automatic regeneration
- **Attack Cooldown**: Balanced firing rate to prevent spam
- **Special Weapons**: Hidden shrines provide powerful temporary abilities:
  - **Thunder Hammer**: Area-of-effect lightning damage
  - **Phoenix Bow**: Triple-arrow spread shots
  - **Shield of Light**: Protective barrier deflecting attacks
  - **Spirit Cannon**: Rapid-fire purple energy blasts
- **Visual Feedback**: Particle explosions, sparkle effects, and screen shake

## Development Status

### ✅ Completed Features
- Complete wave-based survival gameplay
- High score system with persistent storage
- Mobile-responsive design with touch controls
- Comprehensive sound system with background music
- Special weapons system with unique abilities
- Google Analytics integration
- Visual effects and particle systems

### 🚀 Potential Future Enhancements
- Different enemy types with unique behaviors and abilities
- Boss battles at milestone waves (every 10 waves)
- Additional player characters or visual themes
- Environmental hazards and dynamic obstacles
- Power-up drops during gameplay
- Achievement system with unlock rewards
- Multiplayer cooperative or competitive modes

## Browser Compatibility

### Desktop Browsers
- **Chrome 50+**: Full feature support including Web Audio API
- **Firefox 45+**: Complete compatibility with all game systems
- **Safari 10+**: Full support including localStorage and touch events
- **Edge 79+**: Modern Chromium-based Edge with full compatibility

### Mobile Browsers
- **Mobile Chrome**: Optimized touch controls and responsive design
- **Mobile Safari**: Full iOS compatibility with gesture support
- **Mobile Firefox**: Complete mobile feature support
- **Samsung Internet**: Full compatibility on Samsung devices

### Required APIs
- HTML5 Canvas 2D Context
- localStorage API for high score persistence
- Web Audio API for sound system (graceful fallback if unsupported)
- Touch Events API for mobile controls
- RequestAnimationFrame for smooth animation

## Performance
- **Target FPS**: 60 FPS on desktop, optimized for mobile
- **Memory Usage**: Efficient object pooling and cleanup
- **Storage**: Minimal localStorage footprint for high scores
- **Network**: No external dependencies after initial load

## Credits
**Phoenix: Light Warrior** - A comprehensive wave-based survival game featuring:
- Light magic combat system with special weapons
- Mobile-responsive design with touch controls  
- Persistent high score leaderboards
- Heroic fantasy setting suitable for all ages
- Professional sound design and visual effects

Created with modern web technologies and best practices for cross-platform compatibility.