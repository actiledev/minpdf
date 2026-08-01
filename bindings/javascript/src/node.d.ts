declare module "node:fs/promises" {
  export function writeFile(path: string, data: Uint8Array): Promise<void>;
}
