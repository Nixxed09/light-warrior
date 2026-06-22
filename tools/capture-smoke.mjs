import { chromium } from "playwright";
import fs from "node:fs";
import path from "node:path";

const url = process.argv[2] ?? "http://127.0.0.1:3001/";
const outDir = path.resolve("captures");
const outFile = path.join(outDir, "light-warrior-smoke.png");

fs.mkdirSync(outDir, { recursive: true });

const browser = await chromium.launch({ headless: true });
const page = await browser.newPage({ viewport: { width: 1440, height: 900 }, deviceScaleFactor: 1 });

await page.goto(url, { waitUntil: "networkidle" });
await page.screenshot({ path: outFile, fullPage: true });

const canvasPixels = await page.evaluate(() => {
  const canvas = document.querySelector("canvas");
  if (!canvas) return { found: false, nonBlackSamples: 0 };

  const context = canvas.getContext("webgl2") || canvas.getContext("webgl");
  if (!context) return { found: true, nonBlackSamples: -1 };

  const width = Math.max(1, canvas.width);
  const height = Math.max(1, canvas.height);
  const pixels = new Uint8Array(4);
  let nonBlackSamples = 0;
  const points = [
    [Math.floor(width / 2), Math.floor(height / 2)],
    [Math.floor(width / 3), Math.floor(height / 3)],
    [Math.floor((width * 2) / 3), Math.floor(height / 3)],
    [Math.floor(width / 3), Math.floor((height * 2) / 3)],
    [Math.floor((width * 2) / 3), Math.floor((height * 2) / 3)]
  ];

  for (const [x, y] of points) {
    context.readPixels(x, y, 1, 1, context.RGBA, context.UNSIGNED_BYTE, pixels);
    if (pixels[0] + pixels[1] + pixels[2] > 10) nonBlackSamples += 1;
  }

  return { found: true, nonBlackSamples };
});

await browser.close();

console.log(JSON.stringify({ url, outFile, canvasPixels }, null, 2));

if (!canvasPixels.found || canvasPixels.nonBlackSamples === 0) {
  process.exit(1);
}

