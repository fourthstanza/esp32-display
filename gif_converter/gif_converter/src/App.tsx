import { parseGIF, decompressFrames, type ParsedFrame } from "gifuct-js";
import './App.css'

async function loadGif(file: File) {
  const buffer = await file.arrayBuffer();
  const gif = parseGIF(buffer);
  const frames = decompressFrames(gif, true);
  return frames;
}

function frameToImageData(frame: ParsedFrame) {
  const canvas = document.createElement("canvas");
  canvas.width = frame.dims.width;
  canvas.height = frame.dims.height;

  const ctx = canvas.getContext("2d")!;
  const imageData = ctx.createImageData(frame.dims.width, frame.dims.height);
  imageData.data.set(frame.patch);

  ctx.putImageData(imageData, 0, 0);
  return ctx.getImageData(0, 0, frame.dims.width, frame.dims.height);
}

function rgbTo565(r: number, g: number, b: number) {
  return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
}

function imageDataToHex(imageData: ImageData) {
  const hex: string[] = [];

  for (let i = 0; i < imageData.data.length; i += 4) {
    const r = imageData.data[i];
    const g = imageData.data[i + 1];
    const b = imageData.data[i + 2];

    const rgb565 = rgbTo565(r, g, b);
    hex.push(`0x${rgb565.toString(16).padStart(4, "0")}`);
  }

  return hex;
}


function App() {
  <input
    type="file"
    accept="image/gif"
    onChange={handleFile}
    ></input>
}

export default App
