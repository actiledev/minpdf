import createEngine from "./engine.js";

export type Size = { width:number; height:number };
export type Color = [number,number,number];
export type Align = "left"|"center"|"right";
export type Weight = "normal"|"bold"|"italic"|"bolditalic";
export interface TextOptions { size?:number;color?:Color;align?:Align;weight?:Weight;maxWidth?:number;font?:string }
export interface RectOptions { fill?:Color;stroke?:Color;lineWidth?:number;radius?:number }
export interface LineOptions { color?:Color;width?:number;dash?:number[] }
export interface ImageOptions { width?:number;height?:number }
export interface Metadata { title?:string;author?:string;subject?:string;keywords?:string;creator?:string;creationDate?:string }
export interface TableColumn { header:string;width?:number;align?:Align }
export interface TableOptions { columns:TableColumn[];headerBackground?:Color;headerColor?:Color;borderColor?:Color;fontSize?:number;padding?:number }
export const SIZES={A3:{width:842,height:1191},A4:{width:595,height:842},A5:{width:420,height:595},LETTER:{width:612,height:792}} as const;
export const rgb=(r:number,g:number,b:number):Color=>[r/255,g/255,b/255];
const black:Color=[0,0,0];
let enginePromise:Promise<any>|undefined;
const engine=()=>enginePromise??=(createEngine({locateFile:(p:string)=>new URL(p,import.meta.url).href}));

export class PDF {
  #m:any; #doc:number; #closed=false;
  private constructor(m:any,doc:number){this.#m=m;this.#doc=doc}
  static async create(size:Size= SIZES.A4):Promise<PDF>{const m=await engine();const d=m._mpw_create(size.width,size.height);if(!d)throw new Error("minpdf: cannot create document");return new PDF(m,d)}
  #check(code:number){if(code!==0)throw new Error(`minpdf: ${this.#m.UTF8ToString(this.#m._mpw_error(this.#doc))}`)}
  #str(s:string):number{const n=this.#m.lengthBytesUTF8(s)+1,p=this.#m._malloc(n);this.#m.stringToUTF8(s,p,n);return p}
  #strings(values:string[],fn:(ptrs:number[])=>number){const p=values.map(v=>this.#str(v));try{this.#check(fn(p))}finally{p.forEach(x=>this.#m._free(x))}}
  page(size:Size=SIZES.A4){this.#check(this.#m._mpw_page(this.#doc,size.width,size.height));return this}
  text(value:string,x:number,y:number,o:TextOptions={}){const color=o.color??black;this.#strings([value,o.font??""],p=>this.#m._mpw_text(this.#doc,p[0],x,y,o.size??12,...color,["left","center","right"].indexOf(o.align??"left"),["normal","bold","italic","bolditalic"].indexOf(o.weight??"normal"),o.maxWidth??0,p[1]));return this}
  rect(x:number,y:number,width:number,height:number,o:RectOptions={}){const f=o.fill??black,s=o.stroke??black;this.#check(this.#m._mpw_rect(this.#doc,x,y,width,height,...f,...s,o.lineWidth??1,o.radius??0,o.fill?1:0,o.stroke?1:0));return this}
  line(x1:number,y1:number,x2:number,y2:number,o:LineOptions={}){const dash=o.dash??[],p=dash.length?this.#m._malloc(dash.length*4):0;try{if(dash.length)this.#m.HEAPF32.set(dash,p>>>2);this.#check(this.#m._mpw_line(this.#doc,x1,y1,x2,y2,...(o.color??black),o.width??1,p,dash.length))}finally{if(p)this.#m._free(p)}return this}
  circle(cx:number,cy:number,radius:number,o:RectOptions={}){const f=o.fill??black,s=o.stroke??black;this.#check(this.#m._mpw_circle(this.#doc,cx,cy,radius,...f,...s,o.lineWidth??1,o.fill?1:0,o.stroke?1:0));return this}
  image(data:Uint8Array,x:number,y:number,o:ImageOptions={}){const p=this.#m._malloc(data.length);try{this.#m.HEAPU8.set(data,p);this.#check(this.#m._mpw_image(this.#doc,p,data.length,x,y,o.width??0,o.height??0))}finally{this.#m._free(p)}return this}
  registerFont(name:string,data:Uint8Array){const p=this.#m._malloc(data.length),n=this.#str(name);try{this.#m.HEAPU8.set(data,p);this.#check(this.#m._mpw_font(this.#doc,n,p,data.length))}finally{this.#m._free(p);this.#m._free(n)}return this}
  link(label:string,url:string,x:number,y:number,o:{color?:Color;size?:number;underline?:boolean}={}){const c=o.color??[0,0,.933];this.#strings([label,url],p=>this.#m._mpw_link(this.#doc,p[0],p[1],x,y,...c,o.size??12,o.underline===false?0:1));return this}
  metadata(o:Metadata){this.#strings([o.title??"",o.author??"",o.subject??"",o.keywords??"",o.creator??"",o.creationDate??""],p=>this.#m._mpw_metadata(this.#doc,...p));return this}
  table(rows:string[][],x:number,y:number,o:TableOptions){const fs=o.fontSize??10,pad=o.padding??8,widths=o.columns.map((c,i)=>c.width??Math.max(c.header.length,...rows.map(r=>(r[i]??"").length))*fs*.52+2*pad),total=widths.reduce((a,b)=>a+b,0),rh=fs+2*pad;this.rect(x,y,total,rh,{fill:o.headerBackground??rgb(240,240,240)});let cx=x;o.columns.forEach((c,i)=>{const align=c.align??"left",tx=cx+(align==="center"?widths[i]/2:align==="right"?widths[i]-pad:pad);this.text(c.header,tx,y+pad+fs*.8,{size:fs,color:o.headerColor??black,align,weight:"bold"});cx+=widths[i]});let cy=y+rh;rows.forEach((row,ri)=>{this.rect(x,cy,total,rh,{fill:ri%2?rgb(249,249,249):rgb(255,255,255)});cx=x;o.columns.forEach((c,i)=>{const align=c.align??"left",tx=cx+(align==="center"?widths[i]/2:align==="right"?widths[i]-pad:pad);this.text(row[i]??"",tx,cy+pad+fs*.8,{size:fs,align});cx+=widths[i]});cy+=rh});this.rect(x,y,total,rh*(rows.length+1),{stroke:o.borderColor??rgb(204,204,204)});return this}
  bytes():Uint8Array{const n=this.#m._malloc(4);try{const p=this.#m._mpw_build(this.#doc,n);if(!p)this.#check(1);const len=this.#m.HEAPU32[n>>>2],out=this.#m.HEAPU8.slice(p,p+len);this.#m._mpw_free(p);return out}finally{this.#m._free(n)}}
  blob():Blob{return new Blob([this.bytes() as BlobPart],{type:"application/pdf"})}
  objectURL():string{return URL.createObjectURL(this.blob())}
  download(filename="document.pdf"){const a=document.createElement("a"),url=this.objectURL();a.href=url;a.download=filename;a.click();setTimeout(()=>URL.revokeObjectURL(url),0)}
  async save(path:string){const fs=await import("node:fs/promises");await fs.writeFile(path,this.bytes())}
  close(){if(!this.#closed){this.#m._mpw_destroy(this.#doc);this.#closed=true}}
}
