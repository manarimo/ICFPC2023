import { Svg } from "./svg.ts";

export async function serializeSvg(svg: Svg, to: WritableStream) {
    const enc = new TextEncoder();
    const writer = to.getWriter();
    await writer.write(enc.encode(`<svg viewBox="0 0 ${svg.width} ${svg.height}" xmlns="http://www.w3.org/2000/svg">`));

    for (const rect of svg.rects) {
        await writer.write(enc.encode(`<rect x="${rect.x}" y="${rect.y}" width="${rect.width}" height="${rect.height}" fill="${rect.fill || 'none'}" stroke="${rect.stroke || 'none'} "/>`));
    }

    for (const circle of svg.circles) {
        await writer.write(enc.encode(`<circle cx="${circle.x}" cy="${circle.y}" r="${circle.radius}" fill="${circle.fill || 'none'}" stroke="${circle.stroke || 'none'}" />`));
    }

    await writer.write(enc.encode('</svg>'));
}