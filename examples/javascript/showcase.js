import { PDF, SIZES, rgb } from "minpdf";

const png = "path/to/your/image.png"; // Replace with PNG image path

const pdf = await PDF.create(SIZES.A4);
pdf.metadata({ title: "minpdf Javascript", author: "Actile", subject: "Subject here", keywords: "minpdf,pdf,showcase", creator: "minpdf", creationDate: "D:20260101000000Z" });
pdf.text("minpdf Showcase", 48, 62, { size: 26, color: rgb(30, 64, 175), weight: "bold" });
pdf.text("One layout, four language bindings", 48, 84, { size: 11, color: rgb(71, 85, 105) });
pdf.rect(48, 110, 499, 100, { fill: rgb(239, 246, 255), stroke: rgb(147, 197, 253), lineWidth: 1, radius: 10 });
pdf.circle(100, 160, 28, { fill: rgb(37, 99, 235), stroke: rgb(30, 64, 175), lineWidth: 1 });
pdf.text("Shapes, fills, strokes and rounded corners", 148, 150, { size: 14, color: rgb(30, 41, 59), weight: "bold" });
pdf.text("All coordinates and styles match across examples.", 148, 174, { size: 10, color: rgb(71, 85, 105), weight: "italic" });
pdf.line(48, 232, 547, 232, { color: rgb(148, 163, 184), width: 1 });
pdf.text("Team", 48, 264, { size: 16, color: rgb(15, 23, 42), weight: "bold" });
pdf.table([["Ada Lovelace", "Engineer"], ["Grace Hopper", "Admiral"], ["Linus Torvalds", "Maintainer"]], 48, 282, { columns: [{ header: "Name", width: 220 }, { header: "Role", width: 225, align: "right" }] });
pdf.text("Embedded RGBA image", 48, 430, { size: 16, color: rgb(15, 23, 42), weight: "bold" });
pdf.image(png, 48, 450, { width: 64, height: 64 });
pdf.text("The red square is a scaled 1x1 PNG with alpha.", 128, 486, { size: 11, color: rgb(71, 85, 105) });
pdf.link("Visit minpdf documentation", "https://github.com/actiledev/minpdf", 48, 550, { color: rgb(37, 99, 235), underline: true, size: 12 });
pdf.text("Centered text", 297.5, 610, { size: 12, color: rgb(15, 23, 42), align: "center", weight: "bold" });
pdf.text("Right-aligned bold italic", 547, 635, { size: 12, color: rgb(15, 23, 42), align: "right", weight: "bolditalic" });
pdf.page(SIZES.LETTER);
pdf.text("Page 2", 48, 62, { size: 24, color: rgb(30, 64, 175), weight: "bold" });
pdf.text("This Letter-sized page demonstrates mixed page sizes.", 48, 92, { size: 12, color: rgb(51, 65, 85) });
await pdf.save("path/to/your/output.pdf"); // Replace with output path
pdf.close();
