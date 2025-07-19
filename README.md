# Phoenix: Light Warrior

A wave-based survival game where you play as a Warrior of Light defending sacred ground against endless waves of shadow demons in a mystical arena.

## Game Description

In **Phoenix: Light Warrior**, you are the guardian of sacred ground, wielding the power of light magic to fend off increasingly dangerous waves of shadow demons. The game features bright, heroic visuals perfect for players of all ages, with a classic light vs. dark theme.

### Core Gameplay
- **Wave-based Survival**: Survive endless waves of shadow demons that get stronger and more numerous over time
- **Sacred Ground Defense**: Protect the mystical circle at the center of the arena
- **Light Magic Combat**: Click to cast light projectiles that destroy shadow demons
- **Progressive Difficulty**: Each wave brings more enemies with increased speed and power

### Controls
- **WASD** or **Arrow Keys**: Move the Light Warrior
- **Mouse Click**: Cast light magic projectiles toward cursor position
- **Start Game**: Begin defending the sacred ground
- **Pause**: Pause/Resume the action

### Game Features
- Bright, colorful graphics with heroic light vs. dark theme
- Particle effects for magical combat
- Progressive wave system with increasing difficulty
- Score tracking and health management
- Responsive design for different screen sizes

### Visual Style
The game uses a bright, heroic art style suitable for kids featuring:
- Golden light effects and auras
- Vibrant gradient backgrounds
- Glowing magical projectiles
- Shadow demons with menacing red eyes
- Sacred golden circle marking the defended area

### Technical Implementation
- Built with HTML5 Canvas for smooth 2D rendering
- JavaScript game engine with object-oriented design
- CSS animations for UI effects
- Particle system for magical effects
- Collision detection system
- Responsive canvas sizing

## File Structure
```
light-warrior/
├── index.html          # Main game HTML file
├── style.css           # Game styling and visual effects
├── game.js             # Core game logic and classes
├── README.md           # This documentation
└── assets/
    ├── images/         # Future sprite and image assets
    └── sounds/         # Future audio assets
```

## How to Play
1. Open `index.html` in a web browser
2. Click "Start Game" to begin
3. Use WASD or arrow keys to move around the arena
4. Click anywhere to shoot light magic in that direction
5. Destroy shadow demons before they reach you
6. Survive as many waves as possible!

## Game Mechanics

### Health System
- Start with 100 health points
- Lose 20 health when touched by a shadow demon
- Health warning appears when below 30 health
- Game ends when health reaches 0

### Scoring System
- Earn 10 points per enemy destroyed
- Points multiplied by current wave number
- Score displayed in real-time

### Wave Progression
- New wave every 30 seconds
- Each wave increases:
  - Enemy spawn rate
  - Maximum enemies on screen
  - Enemy speed and size

### Combat System
- Light projectiles travel in straight lines
- Projectiles destroy enemies on contact
- Attack cooldown prevents spam clicking
- Particle effects enhance visual feedback

## Future Enhancements
- Sound effects and background music
- Power-ups and special abilities
- Different enemy types with unique behaviors
- Boss battles at certain wave milestones
- High score persistence
- Additional player characters or skins
- Environmental hazards and obstacles

## Browser Compatibility
The game works in all modern browsers that support HTML5 Canvas:
- Chrome 50+
- Firefox 45+
- Safari 10+
- Edge 79+

## Credits
Created as a defensive wave-based survival game featuring light magic combat in a kid-friendly heroic fantasy setting.