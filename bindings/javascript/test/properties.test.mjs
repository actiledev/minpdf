import assert from "node:assert/strict";
import test from "node:test";

import fc from "fast-check";
import { PDF } from "../dist/index.js";

const numRuns = Number.parseInt(process.env.MINPDF_FUZZ_CASES ?? "100", 10);

test("image boundary never traps Wasm", async () => {
  await fc.assert(fc.asyncProperty(fc.uint8Array({ maxLength: 65536 }), async (data) => {
    const pdf = await PDF.create();
    try {
      try {
        pdf.image(data, 0, 0);
      } catch (error) {
        assert.match(String(error), /minpdf:/);
        return;
      }
      assert.equal(Buffer.from(pdf.bytes()).subarray(0, 8).toString(), "%PDF-1.4");
    } finally {
      pdf.close();
    }
  }), { numRuns });
});

test("text boundary never traps Wasm", async () => {
  await fc.assert(fc.asyncProperty(
    fc.string({ maxLength: 256 }),
    fc.float({ min: -10000, max: 10000, noNaN: true }),
    fc.float({ min: -10000, max: 10000, noNaN: true }),
    async (value, x, y) => {
      const pdf = await PDF.create();
      try {
        try {
          pdf.text(value, x, y, { maxWidth: 100 });
        } catch (error) {
          assert.match(String(error), /minpdf:/);
          return;
        }
        assert.match(Buffer.from(pdf.bytes()).toString("latin1"), /xref/);
      } finally {
        pdf.close();
      }
    },
  ), { numRuns });
});

test("font boundary never traps Wasm", async () => {
  await fc.assert(fc.asyncProperty(
    fc.string({ maxLength: 128 }),
    fc.uint8Array({ maxLength: 65536 }),
    async (name, data) => {
      const pdf = await PDF.create();
      try {
        try {
          pdf.registerFont(name, data);
        } catch (error) {
          assert.match(String(error), /minpdf:/);
          return;
        }
        assert.equal(Buffer.from(pdf.bytes()).subarray(0, 8).toString(), "%PDF-1.4");
      } finally {
        pdf.close();
      }
    },
  ), { numRuns });
});
