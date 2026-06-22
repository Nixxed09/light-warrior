import * as THREE from "three";
import "./style.css";

type EnemyKind = "imp" | "stalker" | "berserker" | "warden" | "boss";
type GameState = "menu" | "running" | "won" | "lost";
type WeaponMode = "blade" | "hammer" | "lance";

interface Enemy {
  kind: EnemyKind;
  mesh: THREE.Group;
  speed: number;
  health: number;
  damage: number;
  hitRadius: number;
  contactRadius: number;
  height: number;
  baseScale: number;
  phase: number;
  attackCooldown: number;
  windup: number;
  hitStun: number;
  hitFlash: number;
  warriorHits: number;
}

interface Particle {
  mesh: THREE.Mesh;
  velocity: THREE.Vector3;
  life: number;
  maxLife: number;
}

interface FloatingText {
  sprite: THREE.Sprite;
  velocity: THREE.Vector3;
  life: number;
  maxLife: number;
}

interface TreeResource {
  mesh: THREE.Group;
  size: "small" | "big";
  hits: number;
  hitRadius: number;
  reward: number;
  respawnTimer: number;
}

interface DefenseSegment {
  mesh: THREE.Mesh;
  ring: number;
  index: number;
  radius: number;
  startAngle: number;
  endAngle: number;
  hp: number;
  maxHp: number;
  active: boolean;
}

interface WoodWorker {
  mesh: THREE.Group;
  target: TreeResource | null;
  chopTimer: number;
}

interface AllyWarrior {
  mesh: THREE.Group;
  target: Enemy | null;
  hp: number;
  maxHp: number;
  attackCooldown: number;
}

function requireElement<T extends Element>(selector: string): T {
  const element = document.querySelector<T>(selector);
  if (!element) throw new Error(`Missing required element: ${selector}`);
  return element;
}

const canvas = requireElement<HTMLCanvasElement>("#game");
const startButton = requireElement<HTMLButtonElement>("#start");
const restartButton = requireElement<HTMLButtonElement>("#restart");
const hud = requireElement<HTMLDivElement>("#hud");
const banner = requireElement<HTMLDivElement>("#banner");
const result = requireElement<HTMLDivElement>("#result");
const resultTitle = requireElement<HTMLHeadingElement>("#result-title");
const resultCopy = requireElement<HTMLParagraphElement>("#result-copy");
const healthText = requireElement<HTMLElement>("#health");
const lightText = requireElement<HTMLElement>("#light");
const waveText = requireElement<HTMLElement>("#wave");
const templeText = requireElement<HTMLElement>("#temple");
const weaponText = requireElement<HTMLElement>("#weapon");
const branchesText = requireElement<HTMLElement>("#branches");
const enemyTimerText = requireElement<HTMLElement>("#enemy-timer");
const workersText = requireElement<HTMLElement>("#workers");
const warriorsText = requireElement<HTMLElement>("#warriors");
const levelToast = requireElement<HTMLDivElement>("#level-toast");
const levelTitle = requireElement<HTMLElement>("#level-title");
const levelCopy = requireElement<HTMLElement>("#level-copy");
const bossBar = requireElement<HTMLDivElement>("#boss-bar");
const bossHealthText = requireElement<HTMLElement>("#boss-health-text");
const bossHealthFill = requireElement<HTMLDivElement>("#boss-health-fill");
const damageFlash = requireElement<HTMLDivElement>("#damage-flash");

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x07080b);
scene.fog = new THREE.Fog(0x07080b, 34, 116);

const camera = new THREE.PerspectiveCamera(54, window.innerWidth / window.innerHeight, 0.1, 220);
const renderer = new THREE.WebGLRenderer({ canvas, antialias: true, preserveDrawingBuffer: true });
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.shadowMap.enabled = true;
renderer.shadowMap.type = THREE.PCFSoftShadowMap;
renderer.outputColorSpace = THREE.SRGBColorSpace;
renderer.toneMapping = THREE.ACESFilmicToneMapping;
renderer.toneMappingExposure = 1.08;

const clock = new THREE.Clock();
const keys = new Set<string>();
const playerPosition = new THREE.Vector3(0, 0, 0);
const playerVelocity = new THREE.Vector3();
const moveVector = new THREE.Vector3();
const cameraOffset = new THREE.Vector3();
const arenaRadius = 42;
const templePosition = new THREE.Vector3(0, 0, -31);
const playerRadius = 0.85;
const maxHealth = 200;
const finalLevel = 10;
const enemies: Enemy[] = [];
const particles: Particle[] = [];
const floatingTexts: FloatingText[] = [];
const trees: TreeResource[] = [];
const defenseSegments: DefenseSegment[] = [];
const woodWorkers: WoodWorker[] = [];
const allyWarriors: AllyWarrior[] = [];
const covenantRings: THREE.Mesh[] = [];
const covenantSpokes: THREE.Mesh[] = [];
const restoredSeams: THREE.Mesh[] = [];
const corruptionShards: THREE.Mesh[] = [];

let state: GameState = "menu";
let health = maxHealth;
let lightRadius = 14;
let branches = 0;
let wave = 1;
let killsThisLevel = 0;
let killsNeeded = 6;
let finalBossSpawned = false;
let elapsed = 0;
let spawnTimer = 0;
let dashCooldown = 0;
let dashTimer = 0;
let strikeCooldown = 0;
let hammerTimer = 0;
let lanceCooldown = 0;
let weaponMode: WeaponMode = "blade";
let comboStep = 0;
let comboTimer = 0;
let weaponPoseTimer = 0;
let templeActive = false;
let templePulse = 0;
let cameraYaw = 0;
let isDraggingCamera = false;
let pointerDownX = 0;
let pointerDownY = 0;
let pointerMoved = false;
let attackArcTimer = 0;
let attackArcHit = false;
let attackArcScale = 1;
let attackArcSpin = 7;
let levelTransitionTimer = 0;
let levelTransitionPending = false;
let queuedLevel = 1;
let levelToastTimer = 0;
let playerDamageFlash = 0;
let bossMaxHealth = 1;
let defenseRingCount = 0;
const branchBuildCost = 30;
const workerCost = 10;
const warriorCost = 15;
const defenseSegmentCount = 6;
const defenseSegmentHp = 50;
const enemyAttackCooldown = 3;

const materials = {
  floor: new THREE.MeshStandardMaterial({ color: 0x171723, roughness: 0.88, metalness: 0.06 }),
  corruption: new THREE.MeshStandardMaterial({
    color: 0x170a21,
    emissive: 0x3a0b5f,
    emissiveIntensity: 0.2,
    roughness: 0.82
  }),
  light: new THREE.MeshBasicMaterial({
    color: 0xffe79a,
    transparent: true,
    opacity: 0.34,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  }),
  lightEdge: new THREE.MeshBasicMaterial({
    color: 0xfff0ad,
    transparent: true,
    opacity: 0.78,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  }),
  defense: new THREE.MeshStandardMaterial({
    color: 0xc79b55,
    emissive: 0x3f2b10,
    emissiveIntensity: 0.32,
    roughness: 0.58,
    metalness: 0.18
  }),
  player: new THREE.MeshStandardMaterial({
    color: 0xf6f0df,
    emissive: 0xffd26d,
    emissiveIntensity: 0.56,
    roughness: 0.44,
    metalness: 0.2
  }),
  imp: new THREE.MeshStandardMaterial({
    color: 0x170b25,
    emissive: 0x8217b7,
    emissiveIntensity: 0.52,
    roughness: 0.72
  }),
  stalker: new THREE.MeshStandardMaterial({
    color: 0x101624,
    emissive: 0x7156ff,
    emissiveIntensity: 0.48,
    roughness: 0.62,
    metalness: 0.12
  }),
  berserker: new THREE.MeshStandardMaterial({
    color: 0x270b24,
    emissive: 0xbe1aff,
    emissiveIntensity: 0.58,
    roughness: 0.75
  }),
  warden: new THREE.MeshStandardMaterial({
    color: 0x211f2c,
    emissive: 0x8e54ff,
    emissiveIntensity: 0.55,
    roughness: 0.68,
    metalness: 0.18
  }),
  boss: new THREE.MeshStandardMaterial({
    color: 0x21051c,
    emissive: 0xff245d,
    emissiveIntensity: 0.8,
    roughness: 0.78,
    metalness: 0.08
  }),
  temple: new THREE.MeshStandardMaterial({
    color: 0xbfc6d4,
    emissive: 0x355f8f,
    emissiveIntensity: 0.34,
    roughness: 0.54,
    metalness: 0.2
  }),
  stone: new THREE.MeshStandardMaterial({
    color: 0x7d7680,
    roughness: 0.82,
    metalness: 0.06
  }),
  treeTrunk: new THREE.MeshStandardMaterial({
    color: 0x3b2518,
    roughness: 0.9
  }),
  treeCanopy: new THREE.MeshStandardMaterial({
    color: 0x153627,
    emissive: 0x0b3d22,
    emissiveIntensity: 0.16,
    roughness: 0.82
  }),
  worker: new THREE.MeshStandardMaterial({
    color: 0xd9b28a,
    emissive: 0x2a1608,
    emissiveIntensity: 0.16,
    roughness: 0.72
  }),
  workerAxe: new THREE.MeshStandardMaterial({
    color: 0xc9d3d8,
    roughness: 0.35,
    metalness: 0.45
  }),
  allyWarrior: new THREE.MeshStandardMaterial({
    color: 0xd8ecff,
    emissive: 0x2f89ff,
    emissiveIntensity: 0.28,
    roughness: 0.48,
    metalness: 0.16
  }),
  allySword: new THREE.MeshStandardMaterial({
    color: 0xf7fbff,
    emissive: 0x8fdcff,
    emissiveIntensity: 0.5,
    roughness: 0.24,
    metalness: 0.42
  }),
  gold: new THREE.MeshStandardMaterial({
    color: 0xffd76d,
    emissive: 0xffb84d,
    emissiveIntensity: 0.35,
    roughness: 0.38,
    metalness: 0.45
  }),
  redEye: new THREE.MeshBasicMaterial({
    color: 0xff3156
  }),
  enemyWeapon: new THREE.MeshStandardMaterial({
    color: 0xff6f3d,
    emissive: 0xff2448,
    emissiveIntensity: 0.46,
    roughness: 0.34,
    metalness: 0.28
  }),
  attackArc: new THREE.MeshBasicMaterial({
    color: 0xfff0ad,
    transparent: true,
    opacity: 0,
    depthWrite: false,
    side: THREE.DoubleSide,
    blending: THREE.AdditiveBlending
  }),
  attackArcHit: new THREE.MeshBasicMaterial({
    color: 0x74d7ff,
    transparent: true,
    opacity: 0,
    depthWrite: false,
    side: THREE.DoubleSide,
    blending: THREE.AdditiveBlending
  }),
  lance: new THREE.MeshStandardMaterial({
    color: 0xcff7ff,
    emissive: 0x5adfff,
    emissiveIntensity: 0.9,
    roughness: 0.26,
    metalness: 0.22
  }),
  hammerWeapon: new THREE.MeshStandardMaterial({
    color: 0xd9edff,
    emissive: 0x66c8ff,
    emissiveIntensity: 0.78,
    roughness: 0.34,
    metalness: 0.35
  }),
  covenantLine: new THREE.MeshBasicMaterial({
    color: 0xfff0ad,
    transparent: true,
    opacity: 0.55,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  }),
  covenantBlue: new THREE.MeshBasicMaterial({
    color: 0x9ee7ff,
    transparent: true,
    opacity: 0.36,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  }),
  templePath: new THREE.MeshBasicMaterial({
    color: 0xffd76d,
    transparent: true,
    opacity: 0.22,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  }),
  darkVein: new THREE.MeshBasicMaterial({
    color: 0x6d24ff,
    transparent: true,
    opacity: 0.34,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  })
};

const arena = new THREE.Mesh(
  new THREE.CylinderGeometry(arenaRadius, arenaRadius, 0.8, 96),
  materials.floor
);
arena.receiveShadow = true;
arena.position.y = -0.42;
scene.add(arena);

const arenaRim = new THREE.Mesh(
  new THREE.TorusGeometry(arenaRadius, 0.55, 12, 128),
  materials.stone
);
arenaRim.rotation.x = Math.PI / 2;
arenaRim.position.y = 0.18;
arenaRim.castShadow = true;
arenaRim.receiveShadow = true;
scene.add(arenaRim);

const corruption = new THREE.Mesh(
  new THREE.RingGeometry(18, arenaRadius + 0.8, 128),
  materials.corruption
);
corruption.rotation.x = -Math.PI / 2;
corruption.position.y = 0.015;
scene.add(corruption);

const sacredCircle = new THREE.Mesh(new THREE.CircleGeometry(1, 96), materials.light);
sacredCircle.rotation.x = -Math.PI / 2;
sacredCircle.position.y = 0.04;
scene.add(sacredCircle);

const sacredEdge = new THREE.Mesh(new THREE.RingGeometry(0.97, 1, 128), materials.lightEdge);
sacredEdge.rotation.x = -Math.PI / 2;
sacredEdge.position.y = 0.08;
scene.add(sacredEdge);

const templePath = new THREE.Mesh(new THREE.BoxGeometry(2.2, 0.02, 31), materials.templePath);
templePath.position.set(0, 0.09, -15.5);
scene.add(templePath);

const covenantField = new THREE.Group();
scene.add(covenantField);

for (const ratio of [0.34, 0.58, 0.82]) {
  const ring = new THREE.Mesh(new THREE.RingGeometry(0.985, 1, 128), materials.covenantLine.clone());
  ring.rotation.x = -Math.PI / 2;
  ring.position.y = 0.11 + ratio * 0.025;
  ring.userData.ratio = ratio;
  covenantField.add(ring);
  covenantRings.push(ring);
}

for (let i = 0; i < 16; i += 1) {
  const angle = (i / 16) * Math.PI * 2;
  const spoke = new THREE.Mesh(new THREE.BoxGeometry(1, 0.02, 0.055), materials.covenantBlue.clone());
  spoke.position.y = 0.12;
  spoke.rotation.y = -angle;
  spoke.userData.angle = angle;
  covenantField.add(spoke);
  covenantSpokes.push(spoke);
}

for (let i = 0; i < 18; i += 1) {
  const angle = (i / 18) * Math.PI * 2 + 0.07;
  const seam = new THREE.Mesh(new THREE.BoxGeometry(5.8 + (i % 4) * 1.1, 0.018, 0.07), materials.covenantLine.clone());
  seam.position.y = 0.1;
  seam.rotation.y = -angle;
  seam.userData.angle = angle;
  seam.userData.radius = 7 + (i % 3) * 4.2;
  scene.add(seam);
  restoredSeams.push(seam);
}

function setShadows(object: THREE.Object3D) {
  object.traverse((child) => {
    if (child instanceof THREE.Mesh) {
      child.castShadow = true;
      child.receiveShadow = true;
    }
  });
}

function addArenaLandmarks() {
  for (let i = 0; i < 12; i += 1) {
    const angle = (i / 12) * Math.PI * 2;
    const radius = arenaRadius - 3.2;
    const pillar = new THREE.Group();
    const base = new THREE.Mesh(new THREE.CylinderGeometry(1.25, 1.55, 0.7, 8), materials.stone);
    base.position.y = 0.35;
    pillar.add(base);
    const shaft = new THREE.Mesh(new THREE.CylinderGeometry(0.68, 0.82, 4.4, 8), materials.stone);
    shaft.position.y = 2.9;
    pillar.add(shaft);
    const cap = new THREE.Mesh(new THREE.CylinderGeometry(1.05, 0.82, 0.7, 8), materials.gold);
    cap.position.y = 5.45;
    pillar.add(cap);
    const halo = new THREE.Mesh(new THREE.TorusGeometry(1.04, 0.035, 8, 36), materials.covenantLine.clone());
    halo.position.y = 5.92;
    halo.rotation.x = Math.PI / 2;
    pillar.add(halo);
    const flame = new THREE.PointLight(i % 3 === 0 ? 0x9ee7ff : 0xffce72, 0.75, 14);
    flame.position.y = 6.15;
    pillar.add(flame);
    pillar.position.set(Math.cos(angle) * radius, 0, Math.sin(angle) * radius);
    pillar.lookAt(0, 0, 0);
    setShadows(pillar);
    scene.add(pillar);
  }

  for (let i = 0; i < 20; i += 1) {
    const angle = (i / 20) * Math.PI * 2 + 0.08;
    const radius = 23 + (i % 2) * 5;
    const shard = new THREE.Mesh(new THREE.ConeGeometry(0.45 + (i % 3) * 0.12, 1.8, 5), materials.corruption);
    shard.position.set(Math.cos(angle) * radius, 0.9, Math.sin(angle) * radius);
    shard.rotation.y = -angle;
    shard.rotation.z = (Math.random() - 0.5) * 0.35;
    shard.castShadow = true;
    scene.add(shard);
    corruptionShards.push(shard);

    const vein = new THREE.Mesh(new THREE.BoxGeometry(4.2 + (i % 4) * 0.8, 0.018, 0.08), materials.darkVein.clone());
    vein.position.set(Math.cos(angle) * (radius - 1.3), 0.12, Math.sin(angle) * (radius - 1.3));
    vein.rotation.y = -angle + Math.PI / 2;
    scene.add(vein);
    corruptionShards.push(vein);
  }

  for (let i = 0; i < 30; i += 1) {
    const angle = (i / 30) * Math.PI * 2 + ((i * 17) % 11) * 0.035;
    const ring = i % 3;
    const radius = 13 + ring * 8 + ((i * 7) % 5);
    const isBig = i % 3 === 0;
    const tree = createTree(i, isBig);
    tree.position.set(Math.cos(angle) * radius, 0, Math.sin(angle) * radius);
    tree.rotation.y = -angle + (i % 5) * 0.16;
    setShadows(tree);
    scene.add(tree);
    trees.push({
      mesh: tree,
      size: isBig ? "big" : "small",
      hits: 0,
      hitRadius: isBig ? 1.55 : 1.15,
      reward: isBig ? 3 : 2,
      respawnTimer: 0
    });
  }
}

function createTree(index: number, isBig: boolean) {
  const tree = new THREE.Group();
  const height = (isBig ? 4.15 : 3.15) + (index % 4) * 0.28;
  const trunk = new THREE.Mesh(new THREE.CylinderGeometry(0.18, 0.28, height, 7), materials.treeTrunk);
  trunk.position.y = height / 2;
  tree.add(trunk);

  const lower = new THREE.Mesh(new THREE.ConeGeometry((isBig ? 1.32 : 1.05) + (index % 3) * 0.1, isBig ? 2.55 : 2.1, 8), materials.treeCanopy);
  lower.position.y = height + 0.35;
  tree.add(lower);

  const upper = new THREE.Mesh(new THREE.ConeGeometry((isBig ? 0.98 : 0.78) + (index % 2) * 0.08, isBig ? 2.0 : 1.65, 8), materials.treeCanopy);
  upper.position.y = height + (isBig ? 1.68 : 1.38);
  tree.add(upper);

  const node = new THREE.Mesh(
    new THREE.OctahedronGeometry(isBig ? 0.22 : 0.16),
    new THREE.MeshBasicMaterial({
      color: 0x8df7b5,
      transparent: true,
      opacity: 0.78,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  node.position.y = height + (isBig ? 2.62 : 2.08);
  tree.add(node);

  return tree;
}

function createWoodWorker() {
  const worker = new THREE.Group();
  const body = new THREE.Mesh(new THREE.CapsuleGeometry(0.28, 0.82, 6, 10), materials.worker);
  body.position.y = 0.98;
  worker.add(body);

  const head = new THREE.Mesh(new THREE.SphereGeometry(0.24, 12, 10), materials.worker);
  head.position.y = 1.67;
  worker.add(head);

  const axeHandle = new THREE.Mesh(new THREE.CapsuleGeometry(0.045, 0.92, 5, 8), materials.treeTrunk);
  axeHandle.position.set(0.34, 1.1, -0.16);
  axeHandle.rotation.z = -0.58;
  worker.add(axeHandle);

  const axeHead = new THREE.Mesh(new THREE.BoxGeometry(0.38, 0.16, 0.12), materials.workerAxe);
  axeHead.position.set(0.58, 1.38, -0.22);
  axeHead.rotation.z = -0.58;
  worker.add(axeHead);

  setShadows(worker);
  return worker;
}

function createAllyWarrior() {
  const warrior = new THREE.Group();
  const body = new THREE.Mesh(new THREE.CapsuleGeometry(0.34, 1.05, 7, 12), materials.allyWarrior);
  body.position.y = 1.1;
  warrior.add(body);

  const head = new THREE.Mesh(new THREE.SphereGeometry(0.27, 12, 10), materials.allyWarrior);
  head.position.y = 1.88;
  warrior.add(head);

  const shield = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.72, 0.55), materials.defense);
  shield.position.set(-0.46, 1.12, -0.12);
  warrior.add(shield);

  const sword = new THREE.Mesh(new THREE.BoxGeometry(0.1, 1.15, 0.12), materials.allySword);
  sword.position.set(0.48, 1.16, -0.24);
  sword.rotation.z = -0.42;
  warrior.add(sword);

  const swordTip = new THREE.Mesh(new THREE.ConeGeometry(0.12, 0.28, 6), materials.allySword);
  swordTip.position.set(0.72, 1.66, -0.32);
  swordTip.rotation.z = -0.42;
  warrior.add(swordTip);

  setShadows(warrior);
  return warrior;
}

addArenaLandmarks();

const player = new THREE.Group();
const body = new THREE.Mesh(new THREE.CapsuleGeometry(0.72, 1.45, 8, 14), materials.player);
body.position.y = 1.58;
player.add(body);
const head = new THREE.Mesh(new THREE.SphereGeometry(0.48, 18, 14), materials.player);
head.position.y = 2.72;
player.add(head);
const chestGlow = new THREE.Mesh(
  new THREE.SphereGeometry(0.22, 12, 8),
  new THREE.MeshBasicMaterial({ color: 0xffef9a })
);
chestGlow.position.set(0, 1.9, -0.55);
player.add(chestGlow);
const guardianHalo = new THREE.Mesh(
  new THREE.TorusGeometry(0.72, 0.035, 8, 48),
  new THREE.MeshBasicMaterial({
    color: 0xfff0ad,
    transparent: true,
    opacity: 0.86,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  })
);
guardianHalo.position.y = 3.08;
guardianHalo.rotation.x = Math.PI / 2;
player.add(guardianHalo);
const guardianBackHalo = new THREE.Mesh(
  new THREE.TorusGeometry(1.1, 0.028, 8, 64),
  new THREE.MeshBasicMaterial({
    color: 0x9ee7ff,
    transparent: true,
    opacity: 0.48,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  })
);
guardianBackHalo.position.set(0, 2.05, 0.42);
guardianBackHalo.rotation.x = Math.PI / 2;
player.add(guardianBackHalo);
const shoulderMaterial = new THREE.MeshStandardMaterial({
  color: 0xffd76d,
  emissive: 0xffb84d,
  emissiveIntensity: 0.28,
  roughness: 0.4,
  metalness: 0.35
});
for (const side of [-1, 1]) {
  const shoulder = new THREE.Mesh(new THREE.SphereGeometry(0.28, 12, 10), shoulderMaterial);
  shoulder.position.set(side * 0.68, 2.14, -0.04);
  player.add(shoulder);
  const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.15, 0.78, 5, 8), materials.player);
  arm.position.set(side * 0.82, 1.56, -0.12);
  arm.rotation.z = side * 0.18;
  player.add(arm);
}
const cape = new THREE.Mesh(
  new THREE.BoxGeometry(0.9, 1.6, 0.1),
  new THREE.MeshStandardMaterial({ color: 0x2f4b74, roughness: 0.8 })
);
cape.position.set(0, 1.45, 0.55);
cape.rotation.x = -0.15;
player.add(cape);
const sword = new THREE.Mesh(
  new THREE.BoxGeometry(0.18, 0.18, 2.45),
  new THREE.MeshStandardMaterial({ color: 0xfff0ad, emissive: 0xffd15f, emissiveIntensity: 0.75 })
);
sword.position.set(0.92, 1.46, -0.72);
sword.rotation.x = -0.16;
player.add(sword);

const battleHammer = new THREE.Group();
const battleHammerGrip = new THREE.Mesh(new THREE.CapsuleGeometry(0.09, 1.9, 5, 8), materials.hammerWeapon);
battleHammerGrip.rotation.x = Math.PI / 2;
battleHammer.add(battleHammerGrip);
const battleHammerHead = new THREE.Mesh(new THREE.BoxGeometry(0.98, 0.52, 0.58), materials.hammerWeapon);
battleHammerHead.position.z = -0.98;
battleHammer.add(battleHammerHead);
battleHammer.position.set(0.92, 1.44, -0.72);
battleHammer.rotation.x = -0.12;
battleHammer.visible = false;
player.add(battleHammer);

const lightLance = new THREE.Group();
const lanceShaft = new THREE.Mesh(new THREE.CapsuleGeometry(0.055, 2.65, 5, 10), materials.lance);
lanceShaft.rotation.x = Math.PI / 2;
lightLance.add(lanceShaft);
const lanceTip = new THREE.Mesh(new THREE.ConeGeometry(0.17, 0.58, 8), materials.lance);
lanceTip.position.z = -1.55;
lanceTip.rotation.x = -Math.PI / 2;
lightLance.add(lanceTip);
const lanceGlow = new THREE.PointLight(0x72e6ff, 0.8, 7);
lanceGlow.position.z = -1.4;
lightLance.add(lanceGlow);
lightLance.position.set(0.92, 1.46, -0.72);
lightLance.rotation.x = -0.14;
lightLance.visible = false;
player.add(lightLance);
setShadows(player);
scene.add(player);

const attackArc = new THREE.Mesh(
  new THREE.RingGeometry(1.15, 4.8, 64, 1, -Math.PI * 0.34, Math.PI * 0.68),
  materials.attackArc
);
attackArc.rotation.x = -Math.PI / 2;
attackArc.position.y = 0.18;
attackArc.visible = false;
scene.add(attackArc);

const temple = new THREE.Group();
const templeBase = new THREE.Mesh(new THREE.CylinderGeometry(2.2, 2.7, 0.9, 12), materials.temple);
templeBase.position.y = 0.45;
temple.add(templeBase);
const templeRing = new THREE.Mesh(new THREE.TorusGeometry(2.65, 0.16, 8, 48), materials.gold);
templeRing.rotation.x = Math.PI / 2;
templeRing.position.y = 1.0;
temple.add(templeRing);
const templeGlyph = new THREE.Mesh(new THREE.TorusGeometry(1.42, 0.045, 8, 72), materials.covenantLine.clone());
templeGlyph.rotation.x = Math.PI / 2;
templeGlyph.position.y = 1.22;
temple.add(templeGlyph);
for (let i = 0; i < 6; i += 1) {
  const angle = (i / 6) * Math.PI * 2 + Math.PI / 6;
  const column = new THREE.Mesh(new THREE.CylinderGeometry(0.18, 0.3, 2.45, 8), materials.temple);
  column.position.set(Math.cos(angle) * 1.92, 1.82, Math.sin(angle) * 1.92);
  temple.add(column);
  const cap = new THREE.Mesh(new THREE.ConeGeometry(0.34, 0.68, 8), materials.gold);
  cap.position.set(Math.cos(angle) * 1.92, 3.42, Math.sin(angle) * 1.92);
  temple.add(cap);
}
for (let i = 0; i < 3; i += 1) {
  const angle = (i / 3) * Math.PI * 2 + Math.PI / 2;
  const spire = new THREE.Mesh(new THREE.ConeGeometry(0.32, 2.6, 8), materials.gold);
  spire.position.set(Math.cos(angle) * 1.08, 3.05, Math.sin(angle) * 1.08);
  temple.add(spire);
}
const hammer = new THREE.Mesh(
  new THREE.BoxGeometry(0.7, 2.7, 0.7),
  new THREE.MeshStandardMaterial({ color: 0xaed8ff, emissive: 0x5dbbff, emissiveIntensity: 0.75 })
);
hammer.position.y = 2.35;
temple.add(hammer);
const hammerHead = new THREE.Mesh(
  new THREE.BoxGeometry(2.3, 0.62, 0.95),
  new THREE.MeshStandardMaterial({ color: 0xd8efff, emissive: 0x85d7ff, emissiveIntensity: 0.85 })
);
hammerHead.position.y = 3.65;
temple.add(hammerHead);
const templeLight = new THREE.PointLight(0x6ed8ff, 2.6, 18);
templeLight.position.y = 3.4;
temple.add(templeLight);
const templeGoldLight = new THREE.PointLight(0xffd76d, 1.9, 14);
templeGoldLight.position.y = 1.2;
temple.add(templeGoldLight);
temple.position.copy(templePosition);
setShadows(temple);
scene.add(temple);

const sun = new THREE.DirectionalLight(0xffefd0, 2.6);
sun.position.set(18, 36, 16);
sun.castShadow = true;
sun.shadow.camera.left = -60;
sun.shadow.camera.right = 60;
sun.shadow.camera.top = 60;
sun.shadow.camera.bottom = -60;
scene.add(sun);
scene.add(new THREE.HemisphereLight(0xdde8ff, 0x170b12, 1.6));

window.addEventListener("keydown", (event) => {
  keys.add(event.code);
  if (event.code === "Space") event.preventDefault();
  if (event.code === "KeyE" && !event.repeat && tryBuildDefenseRing()) event.preventDefault();
  if (event.code === "Digit1") setWeaponMode("blade");
  if (event.code === "Digit2") setWeaponMode("hammer");
  if (event.code === "Digit3") setWeaponMode("lance");
  if (event.code === "KeyR" && !event.repeat) hireWoodWorker();
  if (event.code === "KeyT" && !event.repeat) hireAllyWarrior();
  if (event.code === "KeyK") cycleWeapon();
});

window.addEventListener("keyup", (event) => {
  keys.delete(event.code);
});

canvas.addEventListener("pointerdown", (event) => {
  isDraggingCamera = true;
  pointerDownX = event.clientX;
  pointerDownY = event.clientY;
  pointerMoved = false;
  canvas.setPointerCapture(event.pointerId);
});

canvas.addEventListener("pointermove", (event) => {
  if (!isDraggingCamera) return;
  if (Math.abs(event.clientX - pointerDownX) + Math.abs(event.clientY - pointerDownY) > 8) {
    pointerMoved = true;
  }
  cameraYaw -= event.movementX * 0.006;
});

canvas.addEventListener("pointerup", (event) => {
  isDraggingCamera = false;
  canvas.releasePointerCapture(event.pointerId);
  if (!pointerMoved && state === "running") tryStrike();
});

window.addEventListener("resize", () => {
  camera.aspect = window.innerWidth / window.innerHeight;
  camera.updateProjectionMatrix();
  renderer.setSize(window.innerWidth, window.innerHeight);
});

startButton.addEventListener("click", startRun);
restartButton.addEventListener("click", showMainMenu);

function startRun() {
  state = "running";
  hud.classList.remove("hidden");
  banner.classList.add("hidden");
  result.classList.add("hidden");
  health = maxHealth;
  lightRadius = 14;
  branches = 0;
  wave = 1;
  killsThisLevel = 0;
  killsNeeded = getKillsNeeded(wave);
  finalBossSpawned = false;
  levelTransitionTimer = 0;
  levelTransitionPending = false;
  queuedLevel = 1;
  levelToastTimer = 0;
  playerDamageFlash = 0;
  bossMaxHealth = 1;
  defenseRingCount = 0;
  elapsed = 0;
  spawnTimer = 60;
  dashCooldown = 0;
  dashTimer = 0;
  strikeCooldown = 0;
  lanceCooldown = 0;
  comboStep = 0;
  comboTimer = 0;
  weaponPoseTimer = 0;
  attackArcTimer = 0;
  attackArc.visible = false;
  hammerTimer = 0;
  setWeaponMode("blade");
  templeActive = false;
  cameraYaw = 0;
  playerPosition.set(0, 0, 0);
  playerVelocity.set(0, 0, 0);
  enemies.splice(0).forEach((enemy) => scene.remove(enemy.mesh));
  particles.splice(0).forEach((particle) => scene.remove(particle.mesh));
  floatingTexts.splice(0).forEach((text) => scene.remove(text.sprite));
  woodWorkers.splice(0).forEach((worker) => scene.remove(worker.mesh));
  allyWarriors.splice(0).forEach((warrior) => scene.remove(warrior.mesh));
  clearDefenseRings();
  resetTrees();
  bossBar.classList.add("hidden");
  damageFlash.style.opacity = "0";
  showLevelToast("Restoration 1", getLevelPreview(1), 2.2);
  showFloatingText("RESTORATION 1", playerPosition.clone().setY(4), 0xfff0ad);
}

function showMainMenu() {
  state = "menu";
  hud.classList.add("hidden");
  result.classList.add("hidden");
  banner.classList.remove("hidden");
  levelToast.classList.add("hidden");
  bossBar.classList.add("hidden");
  damageFlash.style.opacity = "0";
  enemies.splice(0).forEach((enemy) => scene.remove(enemy.mesh));
  particles.splice(0).forEach((particle) => scene.remove(particle.mesh));
  floatingTexts.splice(0).forEach((text) => scene.remove(text.sprite));
  woodWorkers.splice(0).forEach((worker) => scene.remove(worker.mesh));
  allyWarriors.splice(0).forEach((warrior) => scene.remove(warrior.mesh));
  clearDefenseRings();
  resetTrees();
  health = maxHealth;
  lightRadius = 14;
  branches = 0;
  wave = 1;
  killsThisLevel = 0;
  killsNeeded = getKillsNeeded(wave);
  finalBossSpawned = false;
  levelTransitionPending = false;
  levelTransitionTimer = 0;
  elapsed = 0;
  spawnTimer = 60;
  playerPosition.set(0, 0, 0);
  playerVelocity.set(0, 0, 0);
  player.position.copy(playerPosition);
  updateHud();
}

function spawnEnemy(kind: EnemyKind) {
  const angle = Math.random() * Math.PI * 2;
  const distance = arenaRadius - 4 - Math.random() * 5;
  const profile = getEnemyProfile(kind);
  const mesh = createEnemyMesh(kind);
  mesh.position.set(Math.cos(angle) * distance, 0, Math.sin(angle) * distance);
  scene.add(mesh);
  if (kind === "boss") bossMaxHealth = profile.health;
  enemies.push({
    kind,
    mesh,
    speed: profile.speed,
    health: profile.health,
    damage: profile.damage,
    hitRadius: profile.hitRadius,
    contactRadius: profile.contactRadius,
    height: profile.height,
    baseScale: profile.baseScale,
    phase: Math.random() * Math.PI * 2,
    attackCooldown: 0,
    windup: 0,
    hitStun: 0,
    hitFlash: 0,
    warriorHits: 0
  });
}

function getEnemyProfile(kind: EnemyKind) {
  if (kind === "imp") {
    return {
      speed: 6.25 + wave * 0.24,
      health: 1,
      damage: 5,
      hitRadius: 0.98,
      contactRadius: 0.64,
      height: 2.3,
      baseScale: 1
    };
  }

  if (kind === "stalker") {
    return {
      speed: 7.25 + wave * 0.22,
      health: 2,
      damage: 5,
      hitRadius: 0.82,
      contactRadius: 0.52,
      height: 2.65,
      baseScale: 1
    };
  }

  if (kind === "berserker") {
    return {
      speed: 3.45 + wave * 0.16,
      health: 3 + Math.floor(wave / 3),
      damage: 10,
      hitRadius: 1.44,
      contactRadius: 1.03,
      height: 3.55,
      baseScale: 1
    };
  }

  if (kind === "warden") {
    return {
      speed: 2.85 + wave * 0.1,
      health: 5 + Math.floor(wave / 2),
      damage: 10,
      hitRadius: 1.72,
      contactRadius: 1.16,
      height: 3.8,
      baseScale: 1
    };
  }

  return {
    speed: 2.65,
    health: 28,
    damage: 25,
    hitRadius: 2.48,
    contactRadius: 1.7,
    height: 5.2,
    baseScale: 1.2
  };
}

function createEnemyMesh(kind: EnemyKind) {
  if (kind === "imp") return createImp();
  if (kind === "stalker") return createStalker();
  if (kind === "berserker") return createBerserker();
  if (kind === "warden") return createWarden();
  return createFinalBoss();
}

function getKillsNeeded(level: number) {
  return level >= finalLevel ? 1 : 5 + level * 2;
}

function spawnLevelOpening() {
  if (wave >= finalLevel) {
    spawnEnemy("boss");
    finalBossSpawned = true;
    for (const kind of ["imp", "stalker", "berserker", "warden"] as const) spawnEnemy(kind);
    return;
  }

  const openingCount = Math.min(11, 3 + wave);
  for (let i = 0; i < openingCount; i += 1) {
    spawnEnemy(pickEnemyKind());
  }
}

function pickEnemyKind() {
  const roll = Math.random();
  const stalkerChance = Math.min(0.06 + wave * 0.035, 0.24);
  const wardenChance = wave >= 4 ? Math.min(0.03 + (wave - 3) * 0.035, 0.22) : 0;
  const berserkerChance = Math.min(0.1 + wave * 0.055, 0.44);

  if (roll < wardenChance) return "warden";
  if (roll < wardenChance + stalkerChance) return "stalker";
  if (roll < wardenChance + stalkerChance + berserkerChance) return "berserker";
  return "imp";
}

function createImp() {
  const imp = new THREE.Group();
  const torso = new THREE.Mesh(new THREE.ConeGeometry(0.72, 1.7, 7), materials.imp);
  torso.position.y = 1.02;
  imp.add(torso);
  const head = new THREE.Mesh(new THREE.SphereGeometry(0.42, 12, 10), materials.imp);
  head.position.y = 1.98;
  imp.add(head);
  for (const side of [-1, 1]) {
    const eye = new THREE.Mesh(new THREE.SphereGeometry(0.07, 8, 6), materials.redEye);
    eye.position.set(side * 0.16, 2.04, -0.36);
    imp.add(eye);
    const claw = new THREE.Mesh(new THREE.CapsuleGeometry(0.07, 0.58, 4, 6), materials.imp);
    claw.position.set(side * 0.58, 1.08, -0.08);
    claw.rotation.z = side * 0.52;
    imp.add(claw);

    const dagger = new THREE.Mesh(new THREE.ConeGeometry(0.09, 0.86, 6), materials.enemyWeapon);
    dagger.position.set(side * 0.72, 0.86, -0.34);
    dagger.rotation.set(Math.PI * 0.52, 0, side * -0.42);
    imp.add(dagger);
  }
  const backSpike = new THREE.Mesh(new THREE.ConeGeometry(0.22, 0.75, 5), materials.imp);
  backSpike.position.set(0, 1.45, 0.48);
  backSpike.rotation.x = Math.PI / 2.8;
  imp.add(backSpike);
  setShadows(imp);
  return imp;
}

function createStalker() {
  const stalker = new THREE.Group();
  const body = new THREE.Mesh(new THREE.CapsuleGeometry(0.42, 1.55, 7, 10), materials.stalker);
  body.position.y = 1.32;
  body.rotation.x = 0.16;
  stalker.add(body);

  const mantle = new THREE.Mesh(new THREE.ConeGeometry(0.74, 0.82, 6), materials.stalker);
  mantle.position.y = 2.22;
  mantle.rotation.x = Math.PI;
  stalker.add(mantle);

  const mask = new THREE.Mesh(
    new THREE.BoxGeometry(0.56, 0.34, 0.18),
    new THREE.MeshStandardMaterial({
      color: 0xd8f6ff,
      emissive: 0x6ed8ff,
      emissiveIntensity: 0.55,
      roughness: 0.34,
      metalness: 0.3
    })
  );
  mask.position.set(0, 2.48, -0.34);
  stalker.add(mask);

  const crest = new THREE.Mesh(new THREE.ConeGeometry(0.18, 0.7, 5), materials.gold);
  crest.position.set(0, 2.88, 0.05);
  crest.rotation.x = -0.28;
  stalker.add(crest);

  for (const side of [-1, 1]) {
    const eye = new THREE.Mesh(new THREE.SphereGeometry(0.055, 8, 6), materials.redEye);
    eye.position.set(side * 0.15, 2.5, -0.45);
    stalker.add(eye);

    const leg = new THREE.Mesh(new THREE.CapsuleGeometry(0.1, 1.05, 5, 8), materials.stalker);
    leg.position.set(side * 0.28, 0.58, -0.02);
    leg.rotation.z = side * 0.24;
    stalker.add(leg);

    const blade = new THREE.Mesh(new THREE.ConeGeometry(0.12, 1.22, 5), materials.enemyWeapon);
    blade.position.set(side * 0.62, 1.38, -0.16);
    blade.rotation.z = side * -0.72;
    stalker.add(blade);
  }

  const tail = new THREE.Mesh(new THREE.ConeGeometry(0.12, 1.15, 5), materials.stalker);
  tail.position.set(0, 0.98, 0.64);
  tail.rotation.x = Math.PI / 2.55;
  stalker.add(tail);

  setShadows(stalker);
  return stalker;
}

function createBerserker() {
  const berserker = new THREE.Group();
  const hips = new THREE.Mesh(new THREE.SphereGeometry(0.75, 14, 10), materials.berserker);
  hips.scale.set(1.1, 0.72, 0.9);
  hips.position.y = 0.88;
  berserker.add(hips);
  const torso = new THREE.Mesh(new THREE.DodecahedronGeometry(1.08), materials.berserker);
  torso.scale.set(1.08, 1.28, 0.86);
  torso.position.y = 1.88;
  berserker.add(torso);
  const head = new THREE.Mesh(new THREE.SphereGeometry(0.54, 14, 10), materials.berserker);
  head.position.y = 3.03;
  berserker.add(head);
  for (const side of [-1, 1]) {
    const horn = new THREE.Mesh(new THREE.ConeGeometry(0.16, 0.74, 6), materials.gold);
    horn.position.set(side * 0.38, 3.34, -0.08);
    horn.rotation.z = side * -0.72;
    berserker.add(horn);
    const eye = new THREE.Mesh(new THREE.SphereGeometry(0.08, 8, 6), materials.redEye);
    eye.position.set(side * 0.18, 3.08, -0.44);
    berserker.add(eye);
    const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.18, 1.2, 5, 8), materials.berserker);
    arm.position.set(side * 1.04, 1.82, -0.08);
    arm.rotation.z = side * 0.44;
    berserker.add(arm);
    const claw = new THREE.Mesh(new THREE.ConeGeometry(0.16, 0.8, 5), materials.gold);
    claw.position.set(side * 1.34, 1.08, -0.18);
    claw.rotation.z = side * -0.22;
    berserker.add(claw);

    const axeHandle = new THREE.Mesh(new THREE.CapsuleGeometry(0.06, 1.35, 5, 8), materials.treeTrunk);
    axeHandle.position.set(side * 1.38, 1.18, -0.46);
    axeHandle.rotation.z = side * -0.46;
    berserker.add(axeHandle);

    const axeHead = new THREE.Mesh(new THREE.BoxGeometry(0.46, 0.28, 0.16), materials.enemyWeapon);
    axeHead.position.set(side * 1.62, 1.74, -0.54);
    axeHead.rotation.z = side * -0.46;
    berserker.add(axeHead);

    const leg = new THREE.Mesh(new THREE.CapsuleGeometry(0.22, 0.9, 5, 8), materials.berserker);
    leg.position.set(side * 0.42, 0.48, 0.04);
    leg.rotation.z = side * 0.12;
    berserker.add(leg);
  }
  setShadows(berserker);
  return berserker;
}

function createWarden() {
  const warden = new THREE.Group();
  const base = new THREE.Mesh(new THREE.CylinderGeometry(0.98, 1.2, 0.7, 8), materials.warden);
  base.position.y = 0.44;
  warden.add(base);

  const torso = new THREE.Mesh(new THREE.BoxGeometry(1.7, 1.75, 1.08), materials.warden);
  torso.position.y = 1.55;
  torso.rotation.y = Math.PI / 4;
  warden.add(torso);

  const core = new THREE.Mesh(
    new THREE.OctahedronGeometry(0.42),
    new THREE.MeshBasicMaterial({
      color: 0xc8b2ff,
      transparent: true,
      opacity: 0.92,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  core.position.set(0, 1.72, -0.74);
  warden.add(core);

  const head = new THREE.Mesh(new THREE.DodecahedronGeometry(0.62), materials.warden);
  head.position.y = 2.8;
  head.scale.set(1, 0.78, 1);
  warden.add(head);

  for (const side of [-1, 1]) {
    const shoulder = new THREE.Mesh(new THREE.SphereGeometry(0.38, 10, 8), materials.gold);
    shoulder.position.set(side * 1.0, 2.25, -0.04);
    warden.add(shoulder);

    const hammerArm = new THREE.Mesh(new THREE.CapsuleGeometry(0.2, 1.15, 5, 8), materials.warden);
    hammerArm.position.set(side * 1.26, 1.45, -0.06);
    hammerArm.rotation.z = side * 0.18;
    warden.add(hammerArm);

    const prism = new THREE.Mesh(new THREE.ConeGeometry(0.18, 0.82, 4), materials.gold);
    prism.position.set(side * 1.35, 0.7, -0.12);
    prism.rotation.z = side * Math.PI;
    warden.add(prism);

    const mace = new THREE.Mesh(new THREE.CapsuleGeometry(0.1, 1.25, 5, 8), materials.enemyWeapon);
    mace.position.set(side * 1.58, 1.0, -0.42);
    mace.rotation.z = side * 0.34;
    warden.add(mace);

    const maceHead = new THREE.Mesh(new THREE.DodecahedronGeometry(0.34), materials.enemyWeapon);
    maceHead.position.set(side * 1.78, 0.42, -0.5);
    warden.add(maceHead);

    const eye = new THREE.Mesh(new THREE.SphereGeometry(0.08, 8, 6), materials.redEye);
    eye.position.set(side * 0.16, 2.84, -0.52);
    warden.add(eye);
  }

  const halo = new THREE.Mesh(
    new THREE.TorusGeometry(0.78, 0.045, 8, 36),
    new THREE.MeshBasicMaterial({
      color: 0x9f7cff,
      transparent: true,
      opacity: 0.9,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  halo.position.y = 3.42;
  halo.rotation.x = Math.PI / 2;
  warden.add(halo);

  const aura = new THREE.PointLight(0x8e54ff, 1.45, 10);
  aura.position.y = 2.1;
  warden.add(aura);
  setShadows(warden);
  return warden;
}

function createFinalBoss() {
  const boss = new THREE.Group();
  const base = new THREE.Mesh(new THREE.DodecahedronGeometry(1.8), materials.boss);
  base.scale.set(1.1, 1.35, 0.95);
  base.position.y = 2.0;
  boss.add(base);

  const head = new THREE.Mesh(new THREE.SphereGeometry(0.86, 18, 14), materials.boss);
  head.position.y = 4.0;
  boss.add(head);

  const crown = new THREE.Mesh(new THREE.ConeGeometry(0.78, 1.2, 6), materials.gold);
  crown.position.y = 4.9;
  boss.add(crown);

  for (const side of [-1, 1]) {
    const horn = new THREE.Mesh(new THREE.ConeGeometry(0.22, 1.25, 7), materials.gold);
    horn.position.set(side * 0.74, 4.35, -0.08);
    horn.rotation.z = side * -0.78;
    boss.add(horn);

    const arm = new THREE.Mesh(new THREE.CapsuleGeometry(0.26, 1.9, 6, 10), materials.boss);
    arm.position.set(side * 1.62, 2.25, -0.04);
    arm.rotation.z = side * 0.52;
    boss.add(arm);

    const claw = new THREE.Mesh(new THREE.ConeGeometry(0.26, 1.0, 6), materials.gold);
    claw.position.set(side * 2.05, 1.18, -0.18);
    claw.rotation.z = side * -0.22;
    boss.add(claw);

    const swordBlade = new THREE.Mesh(new THREE.BoxGeometry(0.18, 2.25, 0.18), materials.enemyWeapon);
    swordBlade.position.set(side * 2.18, 1.9, -0.62);
    swordBlade.rotation.z = side * -0.48;
    boss.add(swordBlade);

    const swordTip = new THREE.Mesh(new THREE.ConeGeometry(0.2, 0.48, 6), materials.enemyWeapon);
    swordTip.position.set(side * 2.64, 2.88, -0.72);
    swordTip.rotation.z = side * -0.48;
    boss.add(swordTip);

    const eye = new THREE.Mesh(new THREE.SphereGeometry(0.12, 8, 6), materials.redEye);
    eye.position.set(side * 0.28, 4.08, -0.7);
    boss.add(eye);
  }

  const aura = new THREE.PointLight(0xff2448, 2.8, 16);
  aura.position.y = 3.2;
  boss.add(aura);
  boss.scale.setScalar(1.2);
  setShadows(boss);
  return boss;
}

function update(delta: number) {
  if (state !== "running") {
    updateCamera(delta);
    renderer.render(scene, camera);
    return;
  }

  elapsed += delta;
  dashCooldown = Math.max(0, dashCooldown - delta);
  strikeCooldown = Math.max(0, strikeCooldown - delta);
  lanceCooldown = Math.max(0, lanceCooldown - delta);
  comboTimer = Math.max(0, comboTimer - delta);
  if (comboTimer <= 0) comboStep = 0;
  weaponPoseTimer = Math.max(0, weaponPoseTimer - delta);
  hammerTimer = Math.max(0, hammerTimer - delta);
  templePulse += delta;

  updatePlayer(delta);
  updateThemedScene(delta);
  updateCircle(delta);
  updateTemple(delta);
  updateWeaponPose(weaponPoseTimer > 0);
  updateAttackArc(delta);
  updateProgression(delta);
  updateTrees(delta);
  updateWoodWorkers(delta);
  updateAllyWarriors(delta);
  if (!levelTransitionPending) updateEnemies(delta);
  updateParticles(delta);
  updateFloatingTexts(delta);
  updateUiEffects(delta);
  if (!levelTransitionPending) updateSpawns(delta);
  updateCamera(delta);
  updateHud();
  checkEndState();
  renderer.render(scene, camera);
}

function updatePlayer(delta: number) {
  moveVector.set(0, 0, 0);

  if (keys.has("KeyQ")) cameraYaw += delta * 2.4;
  if (keys.has("KeyE") && playerPosition.length() > lightRadius) cameraYaw -= delta * 2.4;

  const forward = new THREE.Vector3(-Math.sin(cameraYaw), 0, -Math.cos(cameraYaw));
  const right = new THREE.Vector3(Math.cos(cameraYaw), 0, -Math.sin(cameraYaw));
  let forwardInput = 0;
  let rightInput = 0;
  if (keys.has("KeyW") || keys.has("ArrowUp")) forwardInput += 1;
  if (keys.has("KeyS") || keys.has("ArrowDown")) forwardInput -= 1;
  if (keys.has("KeyA") || keys.has("ArrowLeft")) rightInput -= 1;
  if (keys.has("KeyD") || keys.has("ArrowRight")) rightInput += 1;

  moveVector.addScaledVector(forward, forwardInput);
  moveVector.addScaledVector(right, rightInput);
  if (moveVector.lengthSq() > 0) moveVector.normalize();

  if ((keys.has("ShiftLeft") || keys.has("ShiftRight")) && dashCooldown <= 0 && moveVector.lengthSq() > 0) {
    playerVelocity.copy(moveVector).multiplyScalar(27);
    dashCooldown = 0.72;
    dashTimer = 0.16;
    burst(playerPosition, 0xfff0ad, 10, 5);
  }

  const speed = hammerTimer > 0 ? 10.5 : weaponMode === "hammer" ? 7.6 : 8.2;
  if (dashTimer > 0) {
    dashTimer -= delta;
  } else {
    playerVelocity.lerp(moveVector.multiplyScalar(speed), 1 - Math.pow(0.001, delta));
  }

  playerPosition.addScaledVector(playerVelocity, delta);
  if (playerPosition.length() > arenaRadius - 1.6) {
    playerPosition.normalize().multiplyScalar(arenaRadius - 1.6);
  }

  if (playerVelocity.lengthSq() > 0.12) {
    player.rotation.y = Math.atan2(playerVelocity.x, playerVelocity.z);
  }
  player.position.copy(playerPosition);

  if ((keys.has("Space") || keys.has("KeyJ")) && strikeCooldown <= 0) tryStrike();
}

function tryStrike() {
  if (strikeCooldown > 0 || state !== "running") return;
  if (weaponMode === "lance" && lanceCooldown > 0) return;

  if (weaponMode === "hammer") {
    strikeCooldown = hammerTimer > 0 ? 0.5 : 0.68;
  } else if (weaponMode === "lance") {
    strikeCooldown = 0.28;
    lanceCooldown = hammerTimer > 0 ? 0.58 : 0.9;
  } else {
    strikeCooldown = comboStep === 2 ? 0.42 : 0.28;
  }

  strike();
}

function strike() {
  weaponPoseTimer = weaponMode === "hammer" ? 0.24 : weaponMode === "lance" ? 0.18 : 0.14;
  if (weaponMode === "lance") {
    strikeLance();
    return;
  }

  const empowered = hammerTimer > 0;
  const isHammer = weaponMode === "hammer";
  const isComboFinisher = weaponMode === "blade" && comboStep === 2;
  const range = isHammer ? (empowered ? 8.8 : 6.6) : isComboFinisher ? 5.6 : 4.8;
  const closeHitRange = isHammer ? (empowered ? 3.15 : 2.7) : 2.1;
  const damage = isHammer ? (empowered ? 4 : 2) : isComboFinisher ? 2 : 1;
  const knockbackForce = isHammer ? (empowered ? 4.2 : 2.7) : isComboFinisher ? 2.2 : 1.25;
  const hitOrigin = playerPosition.clone();
  const facing = new THREE.Vector3(Math.sin(player.rotation.y), 0, Math.cos(player.rotation.y));
  const forward = getAssistedAttackDirection(hitOrigin, facing, range, closeHitRange);
  player.rotation.y = Math.atan2(forward.x, forward.z);
  const hits: Array<{ enemy: Enemy; distance: number }> = [];
  const arcDot = isHammer ? -0.04 : isComboFinisher ? 0.08 : 0.26;

  for (const enemy of enemies) {
    const toEnemy = enemy.mesh.position.clone().sub(hitOrigin);
    toEnemy.y = 0;
    const distance = toEnemy.length();
    const edgeDistance = Math.max(0, distance - enemy.hitRadius - playerRadius);
    const directionScore = distance > 0 ? toEnemy.clone().normalize().dot(forward) : 1;
    const closeEnough = edgeDistance <= closeHitRange;
    const inMainArc = edgeDistance <= range && directionScore > arcDot;
    if (closeEnough || inMainArc) hits.push({ enemy, distance: edgeDistance });
  }
  hits.sort((a, b) => a.distance - b.distance);
  const treeHits = getTreesInArc(hitOrigin, forward, range, closeHitRange, arcDot);

  showAttackArc(hits.length + treeHits.length > 0, isHammer ? (empowered ? 1.92 : 1.45) : isComboFinisher ? 1.16 : 1, isHammer ? 5 : 9);

  if (isHammer && empowered) {
    burst(playerPosition, 0x74d7ff, 32, 13);
    lightRadius = Math.min(27, lightRadius + 1.3);
  } else if (isHammer) {
    burst(playerPosition.clone().add(forward.clone().multiplyScalar(2.8)), 0x9ee7ff, 18, 9);
  } else {
    burst(hitOrigin.clone().add(forward.clone().multiplyScalar(2.3)), isComboFinisher ? 0xfff0ad : 0xffe59a, isComboFinisher ? 16 : 10, 7);
  }

  for (const { enemy } of hits) {
    damageEnemy(enemy, damage, knockbackForce, isHammer ? 0.28 : 0.16);
  }

  for (const { tree } of treeHits) {
    damageTree(tree);
  }

  if (hits.length === 0 && treeHits.length === 0) {
    showFloatingText(isHammer ? "SMASH" : "SWING", hitOrigin.clone().add(forward.clone().multiplyScalar(2.8)), isHammer ? 0x9ee7ff : 0xfff0ad);
  }

  if (weaponMode === "blade") {
    comboStep = (comboStep + 1) % 3;
    comboTimer = 0.92;
  }
}

function strikeLance() {
  const range = hammerTimer > 0 ? 18 : 14;
  const damage = hammerTimer > 0 ? 3 : 2;
  const hitOrigin = playerPosition.clone();
  const facing = new THREE.Vector3(Math.sin(player.rotation.y), 0, Math.cos(player.rotation.y));
  const forward = getAssistedAttackDirection(hitOrigin, facing, range, 1.4);
  player.rotation.y = Math.atan2(forward.x, forward.z);

  const hits: Array<{ enemy: Enemy; forwardDistance: number; sideDistance: number }> = [];
  for (const enemy of enemies) {
    const toEnemy = enemy.mesh.position.clone().sub(hitOrigin).setY(0);
    const forwardDistance = toEnemy.dot(forward);
    if (forwardDistance < 0.4 || forwardDistance > range + enemy.hitRadius) continue;
    const closest = forward.clone().multiplyScalar(forwardDistance);
    const sideDistance = toEnemy.sub(closest).length();
    if (sideDistance <= enemy.hitRadius + 0.42) hits.push({ enemy, forwardDistance, sideDistance });
  }
  hits.sort((a, b) => a.forwardDistance - b.forwardDistance);
  const pierced = hits.slice(0, hammerTimer > 0 ? 4 : 3);
  const treeHits = getTreesInLance(hitOrigin, forward, range).slice(0, hammerTimer > 0 ? 3 : 2);

  const lastEnemyDistance = pierced.length > 0 ? pierced[pierced.length - 1].forwardDistance : 0;
  const lastTreeDistance = treeHits.length > 0 ? treeHits[treeHits.length - 1].forwardDistance : 0;
  const beamDistance = Math.max(lastEnemyDistance, lastTreeDistance);
  showLanceBeam(hitOrigin, forward, beamDistance > 0 ? Math.min(range, beamDistance + 1.4) : range);
  burst(hitOrigin.clone().add(forward.clone().multiplyScalar(1.5)), 0x74d7ff, 14, 8);

  for (const { enemy } of pierced) {
    damageEnemy(enemy, damage, 1.05, 0.12);
  }

  for (const { tree } of treeHits) {
    damageTree(tree);
  }

  if (pierced.length === 0 && treeHits.length === 0) showFloatingText("LANCE", hitOrigin.clone().add(forward.clone().multiplyScalar(4.2)), 0x74d7ff);
}

function damageEnemy(enemy: Enemy, damage: number, knockbackForce: number, stun: number) {
  enemy.health -= damage;
  const knockback = enemy.mesh.position.clone().sub(playerPosition).setY(0).normalize();
  enemy.mesh.position.addScaledVector(knockback, knockbackForce);
  enemy.hitStun = stun;
  enemy.hitFlash = 0.18;
  showEnemyHit(enemy);
  if (enemy.health <= 0) killEnemy(enemy);
}

function damageEnemyByWarrior(enemy: Enemy) {
  enemy.warriorHits += 1;
  enemy.health -= 3;
  const knockback = enemy.mesh.position.clone().sub(playerPosition).setY(0).normalize();
  enemy.mesh.position.addScaledVector(knockback, 0.72);
  enemy.hitStun = 0.08;
  enemy.hitFlash = 0.18;
  showEnemyHit(enemy);

  if (enemy.kind === "boss") {
    if (enemy.health <= 0) killEnemy(enemy);
    return;
  }

  if (enemy.warriorHits >= 3) {
    killEnemy(enemy);
  } else {
    enemy.health = Math.max(enemy.health, 1);
    showFloatingText(`${enemy.warriorHits}/3`, enemy.mesh.position.clone().setY(enemy.height + 0.6), 0x9ee7ff);
  }
}

function getTreesInArc(origin: THREE.Vector3, forward: THREE.Vector3, range: number, closeHitRange: number, arcDot: number) {
  const hits: Array<{ tree: TreeResource; distance: number }> = [];
  for (const tree of trees) {
    if (!tree.mesh.visible) continue;
    const toTree = tree.mesh.position.clone().sub(origin).setY(0);
    const distance = toTree.length();
    const edgeDistance = Math.max(0, distance - tree.hitRadius - playerRadius);
    const directionScore = distance > 0 ? toTree.clone().normalize().dot(forward) : 1;
    const closeEnough = edgeDistance <= closeHitRange;
    const inMainArc = edgeDistance <= range && directionScore > arcDot;
    if (closeEnough || inMainArc) hits.push({ tree, distance: edgeDistance });
  }
  return hits.sort((a, b) => a.distance - b.distance);
}

function getTreesInLance(origin: THREE.Vector3, forward: THREE.Vector3, range: number) {
  const hits: Array<{ tree: TreeResource; forwardDistance: number; sideDistance: number }> = [];
  for (const tree of trees) {
    if (!tree.mesh.visible) continue;
    const toTree = tree.mesh.position.clone().sub(origin).setY(0);
    const forwardDistance = toTree.dot(forward);
    if (forwardDistance < 0.4 || forwardDistance > range + tree.hitRadius) continue;
    const closest = forward.clone().multiplyScalar(forwardDistance);
    const sideDistance = toTree.sub(closest).length();
    if (sideDistance <= tree.hitRadius + 0.42) hits.push({ tree, forwardDistance, sideDistance });
  }
  return hits.sort((a, b) => a.forwardDistance - b.forwardDistance);
}

function damageTree(tree: TreeResource) {
  if (!tree.mesh.visible) return;
  tree.hits += 1;
  const remaining = Math.max(0, 4 - tree.hits);
  burst(tree.mesh.position.clone().setY(1.2), 0x8ed27d, tree.size === "big" ? 14 : 10, tree.size === "big" ? 7 : 5);
  if (remaining > 0) {
    showFloatingText(`${remaining} TUNES`, tree.mesh.position.clone().setY(tree.size === "big" ? 6.2 : 4.8), 0x8ed27d);
    return;
  }

  branches += tree.reward;
  tree.mesh.visible = false;
  tree.respawnTimer = 15;
  burst(tree.mesh.position.clone().setY(1.6), 0xffd76d, tree.size === "big" ? 30 : 22, tree.size === "big" ? 11 : 8);
  showFloatingText(`+${tree.reward} AETHER`, tree.mesh.position.clone().setY(tree.size === "big" ? 6.2 : 4.8), 0xffd76d);
}

function resetTrees() {
  for (const tree of trees) {
    tree.hits = 0;
    tree.respawnTimer = 0;
    tree.mesh.visible = true;
  }
}

function updateTrees(delta: number) {
  for (const tree of trees) {
    if (tree.mesh.visible || tree.respawnTimer <= 0) continue;
    tree.respawnTimer -= delta;
    if (tree.respawnTimer > 0) continue;
    tree.hits = 0;
    tree.respawnTimer = 0;
    tree.mesh.visible = true;
    burst(tree.mesh.position.clone().setY(1.4), 0x8ed27d, tree.size === "big" ? 22 : 16, tree.size === "big" ? 8 : 6);
    showFloatingText("ROOT NODE RESTORED", tree.mesh.position.clone().setY(tree.size === "big" ? 6.2 : 4.8), 0x8ed27d);
  }
}

function hireWoodWorker() {
  if (state !== "running") return;
  if (branches < workerCost) {
    showFloatingText("NEED 10 AETHER", playerPosition.clone().setY(3.8), 0xffd76d);
    return;
  }

  branches -= workerCost;
  const workerMesh = createWoodWorker();
  const offsetAngle = woodWorkers.length * 1.15;
  workerMesh.position.copy(playerPosition).add(new THREE.Vector3(Math.cos(offsetAngle) * 1.4, 0, Math.sin(offsetAngle) * 1.4));
  scene.add(workerMesh);
  woodWorkers.push({ mesh: workerMesh, target: null, chopTimer: 0.6 });
  burst(workerMesh.position, 0xffd76d, 16, 7);
  showFloatingText("KEEPER", workerMesh.position.clone().setY(2.8), 0xffd76d);
}

function updateWoodWorkers(delta: number) {
  for (const worker of woodWorkers) {
    if (!worker.target || !worker.target.mesh.visible) worker.target = getNearestTree(worker.mesh.position);
    if (!worker.target) continue;

    const toTree = worker.target.mesh.position.clone().sub(worker.mesh.position).setY(0);
    const distance = toTree.length();
    if (distance > 1.75) {
      const direction = toTree.normalize();
      worker.mesh.position.addScaledVector(direction, 4.1 * delta);
      worker.mesh.rotation.y = Math.atan2(direction.x, direction.z);
      continue;
    }

    worker.mesh.lookAt(worker.target.mesh.position.x, worker.mesh.position.y, worker.target.mesh.position.z);
    worker.chopTimer -= delta;
    if (worker.chopTimer <= 0) {
      worker.chopTimer = 1.25;
      damageTree(worker.target);
    }
  }
}

function getNearestTree(origin: THREE.Vector3) {
  let bestTree: TreeResource | null = null;
  let bestDistance = Number.POSITIVE_INFINITY;
  for (const tree of trees) {
    if (!tree.mesh.visible) continue;
    const distance = tree.mesh.position.distanceToSquared(origin);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestTree = tree;
    }
  }
  return bestTree;
}

function hireAllyWarrior() {
  if (state !== "running") return;
  if (branches < warriorCost) {
    showFloatingText("NEED 15 AETHER", playerPosition.clone().setY(3.8), 0x9ee7ff);
    return;
  }

  branches -= warriorCost;
  const warriorMesh = createAllyWarrior();
  const offsetAngle = allyWarriors.length * 1.05 + Math.PI * 0.5;
  warriorMesh.position.copy(playerPosition).add(new THREE.Vector3(Math.cos(offsetAngle) * 1.7, 0, Math.sin(offsetAngle) * 1.7));
  scene.add(warriorMesh);
  allyWarriors.push({ mesh: warriorMesh, target: null, hp: 30, maxHp: 30, attackCooldown: 0.4 });
  burst(warriorMesh.position, 0x9ee7ff, 18, 8);
  showFloatingText("GUARDIAN", warriorMesh.position.clone().setY(3), 0x9ee7ff);
}

function updateAllyWarriors(delta: number) {
  for (const warrior of allyWarriors) {
    warrior.attackCooldown = Math.max(0, warrior.attackCooldown - delta);
    if (!warrior.target || !enemies.includes(warrior.target)) warrior.target = getNearestEnemy(warrior.mesh.position);
    if (!warrior.target) {
      const toPlayer = playerPosition.clone().sub(warrior.mesh.position).setY(0);
      if (toPlayer.length() > 3.2) {
        const direction = toPlayer.normalize();
        warrior.mesh.position.addScaledVector(direction, 4.8 * delta);
        warrior.mesh.rotation.y = Math.atan2(direction.x, direction.z);
      }
      continue;
    }

    const toEnemy = warrior.target.mesh.position.clone().sub(warrior.mesh.position).setY(0);
    const distance = toEnemy.length();
    if (distance > warrior.target.hitRadius + 1.25) {
      const direction = toEnemy.normalize();
      warrior.mesh.position.addScaledVector(direction, 6.1 * delta);
      warrior.mesh.rotation.y = Math.atan2(direction.x, direction.z);
      continue;
    }

    warrior.mesh.lookAt(warrior.target.mesh.position.x, warrior.mesh.position.y, warrior.target.mesh.position.z);
    if (warrior.attackCooldown <= 0) {
      warrior.attackCooldown = 0.8;
      damageEnemyByWarrior(warrior.target);
      burst(warrior.target.mesh.position, 0x9ee7ff, 8, 5);
    }
  }
}

function getNearestEnemy(origin: THREE.Vector3) {
  let bestEnemy: Enemy | null = null;
  let bestDistance = Number.POSITIVE_INFINITY;
  for (const enemy of enemies) {
    const distance = enemy.mesh.position.distanceToSquared(origin);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestEnemy = enemy;
    }
  }
  return bestEnemy;
}

function getNearestWarrior(origin: THREE.Vector3, maxDistance: number) {
  let bestWarrior: AllyWarrior | null = null;
  let bestDistance = maxDistance * maxDistance;
  for (const warrior of allyWarriors) {
    const distance = warrior.mesh.position.distanceToSquared(origin);
    if (distance < bestDistance) {
      bestDistance = distance;
      bestWarrior = warrior;
    }
  }
  return bestWarrior;
}

function damageWarrior(warrior: AllyWarrior, amount: number) {
  warrior.hp -= amount;
  burst(warrior.mesh.position, 0xff3358, 10, 6);
  showFloatingText(`${Math.max(0, Math.ceil(warrior.hp))} HP`, warrior.mesh.position.clone().setY(3), 0xff3358);
  if (warrior.hp > 0) return;

  const index = allyWarriors.indexOf(warrior);
  if (index >= 0) allyWarriors.splice(index, 1);
  scene.remove(warrior.mesh);
  burst(warrior.mesh.position, 0xff3358, 22, 9);
  showFloatingText("GUARDIAN DOWN", warrior.mesh.position.clone().setY(3.2), 0xff3358);
}

function getAssistedAttackDirection(
  origin: THREE.Vector3,
  currentFacing: THREE.Vector3,
  range: number,
  closeHitRange: number
) {
  let bestEnemy: Enemy | null = null;
  let bestScore = Number.POSITIVE_INFINITY;

  for (const enemy of enemies) {
    const toEnemy = enemy.mesh.position.clone().sub(origin).setY(0);
    const distance = toEnemy.length();
    if (distance <= 0) continue;

    const edgeDistance = Math.max(0, distance - enemy.hitRadius - playerRadius);
    const direction = toEnemy.normalize();
    const facingScore = direction.dot(currentFacing);
    const canAssist = edgeDistance <= closeHitRange || (edgeDistance <= range + 0.7 && facingScore > -0.1);
    if (!canAssist) continue;

    const score = edgeDistance - facingScore * 1.2;
    if (score < bestScore) {
      bestScore = score;
      bestEnemy = enemy;
    }
  }

  if (!bestEnemy) return currentFacing.clone().normalize();
  return bestEnemy.mesh.position.clone().sub(origin).setY(0).normalize();
}

function setWeaponMode(mode: WeaponMode) {
  weaponMode = mode;
  updateWeaponPose(false);
}

function cycleWeapon() {
  if (weaponMode === "blade") {
    setWeaponMode("hammer");
  } else if (weaponMode === "hammer") {
    setWeaponMode("lance");
  } else {
    setWeaponMode("blade");
  }
}

function updateWeaponPose(attacking: boolean) {
  sword.visible = weaponMode === "blade";
  battleHammer.visible = weaponMode === "hammer";
  lightLance.visible = weaponMode === "lance";

  const pulse = hammerTimer > 0 ? 1 + Math.sin(elapsed * 13) * 0.08 : 1;
  sword.scale.setScalar(weaponMode === "blade" ? pulse : 1);
  battleHammer.scale.setScalar(weaponMode === "hammer" ? pulse : 1);
  lightLance.scale.setScalar(weaponMode === "lance" ? pulse : 1);

  if (weaponMode === "blade") {
    sword.position.set(0.92, 1.46, attacking ? -0.96 : -0.72);
    sword.rotation.set(attacking ? -0.46 : -0.16, 0, comboStep === 2 ? -0.2 : 0);
  } else if (weaponMode === "hammer") {
    battleHammer.position.set(0.92, 1.42, attacking ? -0.98 : -0.68);
    battleHammer.rotation.set(attacking ? -0.48 : -0.12, 0, attacking ? -0.34 : 0.08);
  } else {
    lightLance.position.set(0.92, 1.46, attacking ? -1.08 : -0.72);
    lightLance.rotation.set(attacking ? -0.24 : -0.14, 0, 0);
  }

  guardianHalo.rotation.z += 0.035;
  guardianBackHalo.rotation.z -= 0.022;
}

function killEnemy(enemy: Enemy) {
  const index = enemies.indexOf(enemy);
  if (index >= 0) enemies.splice(index, 1);
  const isBoss = enemy.kind === "boss";
  burst(enemy.mesh.position, getEnemyBurstColor(enemy.kind), isBoss ? 70 : enemy.kind === "imp" ? 18 : 28, isBoss ? 16 : 9);
  showFloatingText(isBoss ? "DARK CORE BROKEN" : "LIGHT RECLAIMED", enemy.mesh.position.clone().setY(enemy.height + 0.35), isBoss ? 0xfff0ad : getEnemyBurstColor(enemy.kind));
  scene.remove(enemy.mesh);
  lightRadius = Math.min(30, lightRadius + getEnemyLightReward(enemy.kind));

  if (isBoss) {
    showLevelToast("Covenant Restored", "The dark control core falls and the arena lights back up.", 2.5);
    finish("won", "Covenant Restored", "The final boss fell and the luminous guardian field reclaimed the arena.");
    return;
  }

  killsThisLevel += 1;
  if (killsThisLevel >= killsNeeded && wave < finalLevel) advanceLevel();
}

function getEnemyBurstColor(kind: EnemyKind) {
  if (kind === "imp") return 0xffd76d;
  if (kind === "stalker") return 0x74d7ff;
  if (kind === "berserker") return 0xff6f78;
  if (kind === "warden") return 0xb28cff;
  return 0xff2448;
}

function getEnemyLightReward(kind: EnemyKind) {
  if (kind === "imp") return 0.45;
  if (kind === "stalker") return 0.55;
  if (kind === "berserker") return 0.9;
  if (kind === "warden") return 1.25;
  return 5;
}

function advanceLevel() {
  if (levelTransitionPending) return;
  queuedLevel = Math.min(finalLevel, wave + 1);
  levelTransitionPending = true;
  levelTransitionTimer = 2.15;
  killsThisLevel = 0;
  spawnTimer = 99;
  lightRadius = Math.min(30, lightRadius + 2);
  health = Math.min(maxHealth, health + 20);
  enemies.splice(0).forEach((enemy) => {
    burst(enemy.mesh.position, 0xfff0ad, 8, 5);
    scene.remove(enemy.mesh);
  });
  showLevelToast("Node Restored", getLevelPreview(queuedLevel), 2.0);
  showFloatingText("NODE RESTORED", playerPosition.clone().setY(4), 0xfff0ad);
}

function finishLevelTransition() {
  wave = queuedLevel;
  killsThisLevel = 0;
  killsNeeded = getKillsNeeded(wave);
  levelTransitionPending = false;
  levelTransitionTimer = 0;
  spawnTimer = wave >= finalLevel ? 0.7 : 1.0;
  showLevelToast(wave >= finalLevel ? "Dark Control Core" : `Restoration ${wave}`, getLevelPreview(wave), 1.8);
  showFloatingText(wave >= finalLevel ? "DARK CONTROL CORE" : `RESTORATION ${wave}`, playerPosition.clone().setY(4), wave >= finalLevel ? 0xff2448 : 0xfff0ad);
  if (wave >= finalLevel) {
    finalBossSpawned = false;
  } else {
    spawnLevelOpening();
  }
}

function updateProgression(delta: number) {
  if (!levelTransitionPending) return;
  levelTransitionTimer -= delta;
  if (levelTransitionTimer <= 0) finishLevelTransition();
}

function getLevelPreview(level: number) {
  if (level >= finalLevel) return "The dark control core is entering the field. Break it before it retakes the circle.";
  if (level >= 8) return "Wardens and berserkers push containment pressure against the covenant field.";
  if (level >= 5) return "Heavier dark influence enters the arena. Keep restoring ground through brave attacks.";
  if (level >= 3) return "Faster shadows are probing the restored seams.";
  return "Reach the Thunder Hammer temple, then turn brave strikes into reclaimed light.";
}

function showLevelToast(title: string, copy: string, duration: number) {
  levelTitle.textContent = title;
  levelCopy.textContent = copy;
  levelToastTimer = duration;
  levelToast.classList.remove("hidden");
}

function updateUiEffects(delta: number) {
  levelToastTimer = Math.max(0, levelToastTimer - delta);
  if (levelToastTimer <= 0) levelToast.classList.add("hidden");

  playerDamageFlash = Math.max(0, playerDamageFlash - delta);
  damageFlash.style.opacity = `${Math.min(0.75, playerDamageFlash * 2.2)}`;

  const boss = getBossEnemy();
  if (boss && state === "running") {
    const ratio = THREE.MathUtils.clamp(boss.health / Math.max(1, bossMaxHealth), 0, 1);
    bossBar.classList.remove("hidden");
    bossHealthFill.style.width = `${Math.round(ratio * 100)}%`;
    bossHealthText.textContent = `${Math.ceil(ratio * 100)}%`;
  } else {
    bossBar.classList.add("hidden");
  }
}

function getBossEnemy() {
  return enemies.find((enemy) => enemy.kind === "boss");
}

function takePlayerDamage(amount: number) {
  health -= amount;
  playerDamageFlash = 0.34;
  burst(playerPosition, 0xff3358, 12, 6);
  showFloatingText(`-${amount}`, playerPosition.clone().setY(3.4), 0xff3358);
}

function tryBuildDefenseRing() {
  if (state !== "running") return false;
  if (playerPosition.length() > lightRadius) return false;
  if (branches < branchBuildCost) {
    showFloatingText("NEED 30 AETHER", playerPosition.clone().setY(3.8), 0xffd76d);
    return true;
  }

  branches -= branchBuildCost;
  buildDefenseRing();
  showFloatingText(`COVENANT RING ${defenseRingCount}`, playerPosition.clone().setY(4), 0x9ee7ff);
  return true;
}

function buildDefenseRing() {
  defenseRingCount += 1;
  const radius = lightRadius + 1.8 + (defenseRingCount - 1) * 2.15;
  const segmentAngle = (Math.PI * 2) / defenseSegmentCount;
  const gap = 0.12;
  const arcLength = radius * (segmentAngle - gap);

  for (let i = 0; i < defenseSegmentCount; i += 1) {
    const centerAngle = i * segmentAngle + segmentAngle / 2;
    const startAngle = i * segmentAngle + gap / 2;
    const endAngle = (i + 1) * segmentAngle - gap / 2;
    const mesh = new THREE.Mesh(new THREE.BoxGeometry(arcLength, 3.6, 0.9), materials.defense.clone());
    mesh.position.set(Math.cos(centerAngle) * radius, 1.8, Math.sin(centerAngle) * radius);
    mesh.rotation.y = -centerAngle;
    mesh.castShadow = true;
    mesh.receiveShadow = true;
    scene.add(mesh);
    defenseSegments.push({
      mesh,
      ring: defenseRingCount,
      index: i,
      radius,
      startAngle,
      endAngle,
      hp: defenseSegmentHp,
      maxHp: defenseSegmentHp,
      active: true
    });
  }

  burst(playerPosition, 0x9ee7ff, 34, 12);
}

function clearDefenseRings() {
  defenseSegments.splice(0).forEach((segment) => scene.remove(segment.mesh));
}

function getBlockingDefenseSegment(enemy: Enemy) {
  const enemyRadius = enemy.mesh.position.length();
  const angle = normalizeAngle(Math.atan2(enemy.mesh.position.z, enemy.mesh.position.x));
  for (const segment of defenseSegments) {
    if (!segment.active) continue;
    if (Math.abs(enemyRadius - segment.radius) > enemy.contactRadius + 0.55) continue;
    if (!angleInSegment(angle, segment.startAngle, segment.endAngle)) continue;
    return segment;
  }
  return null;
}

function damageDefenseSegment(segment: DefenseSegment, damage: number) {
  segment.hp -= damage;
  const material = segment.mesh.material;
  if (material instanceof THREE.MeshStandardMaterial) {
    material.emissiveIntensity = 0.38;
    material.opacity = Math.max(0.35, segment.hp / segment.maxHp);
    material.transparent = true;
  }
  burst(segment.mesh.position.clone().setY(2.1), 0xffd76d, 8, 5);
  showFloatingText(`${Math.max(0, Math.ceil(segment.hp))} HP`, segment.mesh.position.clone().setY(4.1), 0xffd76d);
  if (segment.hp > 0) return;

  segment.active = false;
  segment.mesh.visible = false;
  burst(segment.mesh.position.clone().setY(2.2), 0xff8a3d, 28, 10);
  showFloatingText("BREACHED", segment.mesh.position.clone().setY(4.3), 0xff3358);
}

function normalizeAngle(angle: number) {
  return (angle + Math.PI * 2) % (Math.PI * 2);
}

function angleInSegment(angle: number, start: number, end: number) {
  const normalizedStart = normalizeAngle(start);
  const normalizedEnd = normalizeAngle(end);
  if (normalizedStart <= normalizedEnd) return angle >= normalizedStart && angle <= normalizedEnd;
  return angle >= normalizedStart || angle <= normalizedEnd;
}

function showAttackArc(hit: boolean, scale: number, spin: number) {
  attackArcTimer = 0.18;
  attackArcHit = hit;
  attackArcScale = scale;
  attackArcSpin = spin;
  attackArc.visible = true;
  attackArc.material = hit ? materials.attackArcHit : materials.attackArc;
  attackArc.position.copy(playerPosition);
  attackArc.position.y = 0.22;
  attackArc.rotation.z = -player.rotation.y + Math.PI * 0.16;
  attackArc.scale.setScalar(attackArcScale);
}

function updateAttackArc(delta: number) {
  if (attackArcTimer <= 0) {
    attackArc.visible = false;
    return;
  }

  attackArcTimer -= delta;
  attackArc.position.copy(playerPosition);
  attackArc.position.y = 0.22;
  attackArc.rotation.z += delta * (attackArcHit ? attackArcSpin * 1.18 : attackArcSpin);
  attackArc.scale.setScalar(attackArcScale * (1 + (1 - attackArcTimer / 0.18) * 0.18));
  const alpha = Math.max(0, attackArcTimer / 0.18);
  const material = attackArc.material;
  if (material instanceof THREE.MeshBasicMaterial) material.opacity = attackArcHit ? alpha * 0.9 : alpha * 0.58;
  if (attackArcTimer <= 0) attackArc.visible = false;
}

function showLanceBeam(origin: THREE.Vector3, forward: THREE.Vector3, length: number) {
  const midpoint = origin.clone().add(forward.clone().multiplyScalar(length / 2));
  const beam = new THREE.Mesh(
    new THREE.BoxGeometry(0.18, 0.18, length),
    new THREE.MeshBasicMaterial({
      color: 0x8eeaff,
      transparent: true,
      opacity: 0.9,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  beam.position.copy(midpoint);
  beam.position.y = 1.15;
  beam.rotation.y = player.rotation.y;
  scene.add(beam);
  particles.push({
    mesh: beam,
    velocity: new THREE.Vector3(0, 1.2, 0),
    life: 0.18,
    maxLife: 0.18
  });

  const flare = new THREE.Mesh(
    new THREE.SphereGeometry(0.34, 12, 8),
    new THREE.MeshBasicMaterial({
      color: 0xd8fbff,
      transparent: true,
      opacity: 0.95,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  flare.position.copy(origin).add(forward.clone().multiplyScalar(length));
  flare.position.y = 1.15;
  scene.add(flare);
  particles.push({
    mesh: flare,
    velocity: new THREE.Vector3(0, 0.9, 0),
    life: 0.22,
    maxLife: 0.22
  });
}

function showEnemyHit(enemy: Enemy) {
  burst(enemy.mesh.position, getEnemyBurstColor(enemy.kind), enemy.kind === "imp" ? 10 : 16, 6);
  const marker = new THREE.Mesh(
    new THREE.TorusGeometry(enemy.hitRadius, 0.045, 8, 36),
    new THREE.MeshBasicMaterial({
      color: 0xfff0ad,
      transparent: true,
      opacity: 0.95,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  marker.position.copy(enemy.mesh.position);
  marker.position.y = enemy.height * 0.58;
  marker.rotation.x = Math.PI / 2;
  scene.add(marker);
  particles.push({
    mesh: marker,
    velocity: new THREE.Vector3(0, 0.8, 0),
    life: 0.28,
    maxLife: 0.28
  });
  if (enemy.health > 0) showFloatingText("HIT", enemy.mesh.position.clone().setY(enemy.height + 0.2), 0xffffff);
}

function showEnemyAttack(enemy: Enemy) {
  const toPlayer = playerPosition.clone().sub(enemy.mesh.position).setY(0).normalize();
  const midpoint = enemy.mesh.position.clone().add(toPlayer.multiplyScalar(enemy.hitRadius + 0.65));
  const slash = new THREE.Mesh(
    new THREE.TorusGeometry(enemy.kind === "boss" ? 1.55 : enemy.kind === "imp" ? 0.62 : 0.95, 0.045, 8, 36, Math.PI * 1.1),
    new THREE.MeshBasicMaterial({
      color: 0xff3358,
      transparent: true,
      opacity: 0.92,
      depthWrite: false,
      blending: THREE.AdditiveBlending
    })
  );
  slash.position.copy(midpoint);
  slash.position.y = enemy.kind === "boss" ? 2.8 : enemy.kind === "imp" ? 1.25 : 1.8;
  slash.rotation.x = Math.PI / 2;
  slash.rotation.z = enemy.mesh.rotation.y;
  scene.add(slash);
  particles.push({
    mesh: slash,
    velocity: new THREE.Vector3(0, 1.1, 0),
    life: 0.24,
    maxLife: 0.24
  });
  showFloatingText("ATTACK", playerPosition.clone().setY(3.7), 0xff3358);
}

function showFloatingText(label: string, origin: THREE.Vector3, color: number) {
  const canvas = document.createElement("canvas");
  canvas.width = 512;
  canvas.height = 128;
  const context = canvas.getContext("2d");
  if (!context) return;

  const hex = `#${color.toString(16).padStart(6, "0")}`;
  context.clearRect(0, 0, canvas.width, canvas.height);
  context.font = "900 54px system-ui, sans-serif";
  context.textAlign = "center";
  context.textBaseline = "middle";
  context.lineWidth = 9;
  context.strokeStyle = "rgba(0, 0, 0, 0.78)";
  context.strokeText(label, 256, 64);
  context.fillStyle = hex;
  context.fillText(label, 256, 64);

  const texture = new THREE.CanvasTexture(canvas);
  texture.colorSpace = THREE.SRGBColorSpace;
  const material = new THREE.SpriteMaterial({
    map: texture,
    transparent: true,
    depthWrite: false
  });
  const sprite = new THREE.Sprite(material);
  sprite.position.copy(origin);
  sprite.scale.set(3.2, 0.8, 1);
  scene.add(sprite);
  floatingTexts.push({
    sprite,
    velocity: new THREE.Vector3(0, 2.3, 0),
    life: 0.72,
    maxLife: 0.72
  });
}

function updateThemedScene(delta: number) {
  covenantField.rotation.y += delta * 0.035;
  templePath.rotation.y = Math.sin(elapsed * 0.35) * 0.012;

  const outside = playerPosition.length() > lightRadius;
  const guardianPulse = 1 + Math.sin(elapsed * 6.5) * 0.055 + (hammerTimer > 0 ? 0.1 : 0);
  guardianHalo.scale.setScalar(guardianPulse);
  guardianBackHalo.scale.setScalar(guardianPulse * (outside ? 0.88 : 1));

  for (const ring of covenantRings) {
    const ratio = ring.userData.ratio as number;
    ring.scale.setScalar(lightRadius * ratio);
    const material = ring.material;
    if (material instanceof THREE.MeshBasicMaterial) {
      material.opacity = (outside ? 0.32 : 0.48) + Math.sin(elapsed * (2.2 + ratio) + ratio * 8) * 0.08;
    }
  }

  for (const spoke of covenantSpokes) {
    const angle = spoke.userData.angle as number;
    const length = lightRadius * 0.88;
    spoke.scale.set(length, 1, 1);
    spoke.position.set(Math.cos(angle) * length * 0.5, 0.12, Math.sin(angle) * length * 0.5);
    const material = spoke.material;
    if (material instanceof THREE.MeshBasicMaterial) {
      material.opacity = outside ? 0.2 : 0.28 + Math.sin(elapsed * 3 + angle) * 0.08;
    }
  }

  for (const seam of restoredSeams) {
    const angle = seam.userData.angle as number;
    const radius = seam.userData.radius as number;
    const reclaimed = radius < lightRadius - 1;
    seam.position.set(Math.cos(angle) * radius, 0.1, Math.sin(angle) * radius);
    const material = seam.material;
    if (material instanceof THREE.MeshBasicMaterial) {
      material.opacity = reclaimed ? 0.48 + Math.sin(elapsed * 4 + angle) * 0.12 : 0.08;
    }
  }

  for (let i = 0; i < corruptionShards.length; i += 1) {
    const shard = corruptionShards[i];
    shard.scale.y = 1 + Math.sin(elapsed * 2.8 + i) * 0.08;
    const material = shard instanceof THREE.Mesh ? shard.material : null;
    if (material instanceof THREE.MeshBasicMaterial) {
      material.opacity = playerPosition.length() > lightRadius ? 0.48 : 0.28 + Math.sin(elapsed * 2 + i) * 0.06;
    }
  }

  const pathMaterial = templePath.material;
  if (pathMaterial instanceof THREE.MeshBasicMaterial) {
    const pathRead = playerPosition.distanceTo(templePosition) < 16 || hammerTimer > 0;
    pathMaterial.opacity = pathRead ? 0.42 + Math.sin(elapsed * 5) * 0.08 : 0.22;
  }
}

function updateCircle(delta: number) {
  const outside = playerPosition.length() > lightRadius;
  if (outside) {
    health -= 10 * delta;
  }

  sacredCircle.scale.setScalar(lightRadius);
  sacredEdge.scale.setScalar(lightRadius);
  const edgePulse = 0.78 + Math.sin(elapsed * 5) * 0.08;
  materials.lightEdge.opacity = outside ? 0.96 : edgePulse;
  materials.light.opacity = outside ? 0.2 : 0.3;
  corruption.scale.setScalar(Math.max(1, lightRadius / 18));
}

function updateTemple(delta: number) {
  temple.rotation.y += delta * 0.55;
  const distance = playerPosition.distanceTo(templePosition);
  if (distance < 3.5 && !templeActive) {
    templeActive = true;
    hammerTimer = 14;
    lightRadius = Math.min(24, lightRadius + 4);
    burst(templePosition, 0x78d8ff, 52, 16);
  }

  if (hammerTimer <= 0) templeActive = false;
  const pulse = 1 + Math.sin(templePulse * 4.8) * (templeActive ? 0.18 : 0.08);
  temple.scale.setScalar(pulse);
}

function updateEnemies(delta: number) {
  for (const enemy of [...enemies]) {
    enemy.attackCooldown = Math.max(0, enemy.attackCooldown - delta);
    enemy.hitStun = Math.max(0, enemy.hitStun - delta);
    enemy.hitFlash = Math.max(0, enemy.hitFlash - delta);
    const flashScale = enemy.hitFlash > 0 ? 1 + Math.sin(enemy.hitFlash * 80) * 0.08 : 1;

    const nearestWarrior = getNearestWarrior(enemy.mesh.position, 7.5);
    const targetPosition = nearestWarrior?.mesh.position ?? playerPosition;
    const toTarget = targetPosition.clone().sub(enemy.mesh.position).setY(0);
    const distance = toTarget.length();
    const direction = distance > 0 ? toTarget.normalize() : toTarget;
    const contactDistance = playerRadius + enemy.contactRadius;
    const preferredDistance = contactDistance + 0.35;
    const blockingSegment = getBlockingDefenseSegment(enemy);

    if (canLunge(enemy.kind) && distance < getLungeRange(enemy.kind) && enemy.windup <= 0 && enemy.attackCooldown <= 0) {
      enemy.windup = enemy.kind === "boss" ? 0.62 : enemy.kind === "warden" ? 0.5 : 0.42;
    }

    const bob = Math.sin(elapsed * (enemy.kind === "stalker" ? 9 : 5.5) + enemy.phase) * (enemy.kind === "stalker" ? 0.035 : 0.02);
    const base = enemy.baseScale + bob;
    if (enemy.hitStun > 0) {
      enemy.mesh.scale.lerp(new THREE.Vector3(base * 1.08 * flashScale, base * 0.92 * flashScale, base * 1.08 * flashScale), 0.45);
    } else if (blockingSegment) {
      enemy.mesh.scale.lerp(new THREE.Vector3(base, base, base), 0.2);
      const awayFromCenter = enemy.mesh.position.clone().setY(0).normalize();
      enemy.mesh.position.copy(awayFromCenter.multiplyScalar(blockingSegment.radius + enemy.contactRadius + 0.62));
      if (enemy.attackCooldown <= 0) {
        damageDefenseSegment(blockingSegment, enemy.damage);
        enemy.attackCooldown = enemyAttackCooldown;
      }
    } else if (enemy.windup > 0) {
      enemy.windup -= delta;
      enemy.mesh.scale.setScalar(base * (1 + Math.sin(enemy.windup * 40) * 0.08));
      if (enemy.windup <= 0) enemy.mesh.position.addScaledVector(direction, getLungeDistance(enemy.kind));
    } else {
      enemy.mesh.scale.lerp(new THREE.Vector3(base, base, base), 0.2);
      if (distance > preferredDistance) {
        enemy.mesh.position.addScaledVector(direction, enemy.speed * delta * (1 + (wave - 1) * 0.07));
      } else if (distance < contactDistance) {
        enemy.mesh.position.addScaledVector(direction, -enemy.speed * delta * 0.8);
      }
    }

    enemy.mesh.lookAt(targetPosition.x, enemy.mesh.position.y, targetPosition.z);

    const currentAttackDistance = enemy.mesh.position.clone().sub(targetPosition).setY(0).length();
    if (currentAttackDistance < contactDistance + 0.18 && enemy.attackCooldown <= 0) {
      if (nearestWarrior) {
        damageWarrior(nearestWarrior, enemy.damage);
      } else {
        takePlayerDamage(enemy.damage);
      }
      enemy.attackCooldown = enemyAttackCooldown;
      showEnemyAttack(enemy);
      const shove = enemy.mesh.position.clone().sub(targetPosition).setY(0).normalize();
      enemy.mesh.position.addScaledVector(shove, 2.5);
    }
  }
}

function canLunge(kind: EnemyKind) {
  return kind === "berserker" || kind === "warden" || kind === "boss";
}

function getLungeRange(kind: EnemyKind) {
  if (kind === "boss") return 16;
  if (kind === "warden") return 14;
  return 13;
}

function getLungeDistance(kind: EnemyKind) {
  if (kind === "boss") return 4.2;
  if (kind === "warden") return 4.8;
  return 5.8;
}

function updateSpawns(delta: number) {
  if (levelTransitionPending) return;
  spawnTimer -= delta;
  if (spawnTimer <= 0) {
    if (wave >= finalLevel) {
      if (!finalBossSpawned) {
        spawnEnemy("boss");
        finalBossSpawned = true;
      } else if (enemies.length < 9) {
        spawnEnemy(pickEnemyKind());
      }
      spawnTimer = 1.25;
      return;
    }

    const maxEnemies = Math.min(34, 7 + wave * 3);
    if (enemies.length < maxEnemies) {
      spawnEnemy(pickEnemyKind());
    }
    spawnTimer = Math.max(0.38, 1.55 - wave * 0.105);
  }
}

function burst(origin: THREE.Vector3, color: number, count: number, force: number) {
  const material = new THREE.MeshBasicMaterial({
    color,
    transparent: true,
    opacity: 0.9,
    depthWrite: false,
    blending: THREE.AdditiveBlending
  });
  for (let i = 0; i < count; i += 1) {
    const mesh = new THREE.Mesh(new THREE.SphereGeometry(0.12 + Math.random() * 0.12, 8, 8), material.clone());
    mesh.position.copy(origin);
    mesh.position.y += 0.7 + Math.random() * 1.2;
    scene.add(mesh);
    particles.push({
      mesh,
      velocity: new THREE.Vector3(
        (Math.random() - 0.5) * force,
        Math.random() * force * 0.65,
        (Math.random() - 0.5) * force
      ),
      life: 0.55 + Math.random() * 0.42,
      maxLife: 0.9
    });
  }
}

function updateParticles(delta: number) {
  for (const particle of [...particles]) {
    particle.life -= delta;
    particle.velocity.y -= 10 * delta;
    particle.mesh.position.addScaledVector(particle.velocity, delta);
    const material = particle.mesh.material;
    if (material instanceof THREE.MeshBasicMaterial) {
      material.opacity = Math.max(0, particle.life / particle.maxLife);
    }
    if (particle.life <= 0) {
      const index = particles.indexOf(particle);
      if (index >= 0) particles.splice(index, 1);
      scene.remove(particle.mesh);
    }
  }
}

function updateFloatingTexts(delta: number) {
  for (const text of [...floatingTexts]) {
    text.life -= delta;
    text.sprite.position.addScaledVector(text.velocity, delta);
    const material = text.sprite.material;
    if (material instanceof THREE.SpriteMaterial) {
      material.opacity = Math.max(0, text.life / text.maxLife);
    }
    if (text.life <= 0) {
      const index = floatingTexts.indexOf(text);
      if (index >= 0) floatingTexts.splice(index, 1);
      const material = text.sprite.material;
      if (material instanceof THREE.SpriteMaterial) material.map?.dispose();
      scene.remove(text.sprite);
    }
  }
}

function updateCamera(delta: number) {
  const cameraDistance = window.innerWidth < 720 ? 26 : 24;
  const cameraHeight = window.innerWidth < 720 ? 15 : 14;
  cameraOffset.set(Math.sin(cameraYaw) * cameraDistance, cameraHeight, Math.cos(cameraYaw) * cameraDistance);
  const lookAhead = new THREE.Vector3(-Math.sin(cameraYaw), 0, -Math.cos(cameraYaw)).multiplyScalar(4.8);
  const desired = playerPosition.clone().add(cameraOffset);
  camera.position.lerp(desired, 1 - Math.pow(0.0001, delta));
  const target = playerPosition.clone().add(lookAhead);
  camera.lookAt(target.x, 1.55, target.z);
}

function updateHud() {
  healthText.textContent = `${Math.max(0, Math.ceil(health))}`;
  lightText.textContent = `${Math.round(lightRadius)}m`;
  waveText.textContent = wave >= finalLevel ? `${wave}/${finalLevel} Boss` : `${wave}/${finalLevel} (${killsThisLevel}/${killsNeeded})`;
  templeText.textContent = hammerTimer > 0 ? `${Math.ceil(hammerTimer)}s` : templeActive ? "Ready" : "North";
  weaponText.textContent = getWeaponHudText();
  branchesText.textContent = `${branches}`;
  enemyTimerText.textContent = getEnemyTimerText();
  workersText.textContent = `${woodWorkers.length}`;
  warriorsText.textContent = `${allyWarriors.length}`;
}

function getEnemyTimerText() {
  if (state !== "running") return "60s";
  if (levelTransitionPending) return "Hold";
  if (wave >= finalLevel) return finalBossSpawned ? "Boss" : `${Math.max(0, Math.ceil(spawnTimer))}s`;
  if (killsThisLevel > 0 || enemies.length > 0 || elapsed >= 60) return "Active";
  return `${Math.max(0, Math.ceil(spawnTimer))}s`;
}

function getWeaponHudText() {
  const empowered = hammerTimer > 0 ? "*" : "";
  if (weaponMode === "blade") {
    return `Blade ${comboStep + 1}/3${empowered}`;
  }
  if (weaponMode === "hammer") {
    return `Hammer${empowered}`;
  }
  if (lanceCooldown > 0) {
    return `Lance ${lanceCooldown.toFixed(1)}s`;
  }
  return `Lance${empowered}`;
}

function checkEndState() {
  if (health <= 0) {
    finish("lost", "Defeated", "You have been corrupted by the darkness.");
  }
}

function finish(nextState: GameState, title: string, copy: string) {
  state = nextState;
  hud.classList.add("hidden");
  resultTitle.textContent = title;
  resultCopy.textContent = copy;
  bossBar.classList.add("hidden");
  result.classList.remove("hidden");
}

function loop() {
  update(Math.min(clock.getDelta(), 0.033));
  requestAnimationFrame(loop);
}

updateHud();
loop();
