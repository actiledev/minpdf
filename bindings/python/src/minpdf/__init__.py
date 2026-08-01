"""Fast, zero-runtime-dependency PDF generation."""
from __future__ import annotations
from dataclasses import dataclass
from pathlib import Path
from typing import Self
import _minpdf

Color = tuple[float, float, float]
A3 = (842.0, 1191.0)
A4 = (595.0, 842.0)
A5 = (420.0, 595.0)
LETTER = (612.0, 792.0)
BLACK: Color = (0.0, 0.0, 0.0)

def rgb(r: int, g: int, b: int) -> Color:
    return (r / 255, g / 255, b / 255)

@dataclass(slots=True)
class TextOptions:
    size: float = 12
    color: Color = BLACK
    align: str = "left"
    weight: str = "normal"
    max_width: float = 0
    font: str = ""

@dataclass(slots=True)
class RectOptions:
    fill: Color | None = None
    stroke: Color | None = None
    line_width: float = 1
    radius: float = 0

@dataclass(slots=True)
class ImageOptions:
    width: float = 0
    height: float = 0

@dataclass(slots=True)
class TableColumn:
    header: str
    width: float = 0
    align: str = "left"

class PDF:
    def __init__(self, size: tuple[float, float] = A4):
        self._doc = _minpdf.create(*size)

    def __enter__(self) -> Self: return self
    def __exit__(self, *_: object) -> None: self._doc = None
    def page(self, size: tuple[float, float] = A4) -> Self:
        _minpdf.page(self._doc, *size); return self
    def text(self, value: str, x: float, y: float, options: TextOptions | None = None) -> Self:
        o = options or TextOptions(); _minpdf.text(self._doc,value,x,y,o.size,*o.color,o.max_width,{"left":0,"center":1,"right":2}[o.align],{"normal":0,"bold":1,"italic":2,"bolditalic":3}[o.weight],o.font); return self
    def rect(self, x: float, y: float, width: float, height: float, options: RectOptions | None = None) -> Self:
        o=options or RectOptions();f=o.fill or BLACK;s=o.stroke or BLACK;_minpdf.rect(self._doc,x,y,width,height,*f,*s,o.line_width,o.radius,o.fill is not None,o.stroke is not None);return self
    def line(self,x1:float,y1:float,x2:float,y2:float,color:Color=BLACK,width:float=1,dash:tuple[float,...]|list[float]=()) -> Self:
        _minpdf.line(self._doc,x1,y1,x2,y2,*color,width,dash);return self
    def circle(self,cx:float,cy:float,radius:float,options:RectOptions|None=None) -> Self:
        o=options or RectOptions();f=o.fill or BLACK;s=o.stroke or BLACK;_minpdf.circle(self._doc,cx,cy,radius,*f,*s,o.line_width,o.fill is not None,o.stroke is not None);return self
    def image(self,data:bytes,x:float,y:float,options:ImageOptions|None=None) -> Self:
        o=options or ImageOptions();_minpdf.image(self._doc,data,x,y,o.width,o.height);return self
    def register_font(self,name:str,data:bytes) -> Self:
        _minpdf.register_font(self._doc,name,data);return self
    def link(self,label:str,url:str,x:float,y:float,color:Color=(0,0,.933),underline:bool=True,size:float=12) -> Self:
        _minpdf.link(self._doc,label,url,x,y,*color,size,underline);return self
    def metadata(self,*,title:str="",author:str="",subject:str="",keywords:str="",creator:str="",creation_date:str="") -> Self:
        _minpdf.metadata(self._doc,title,author,subject,keywords,creator,creation_date);return self
    def table(self, rows:list[list[str]], x:float, y:float, columns:list[TableColumn], *, font_size:float=10, padding:float=8, header_background:Color=rgb(240,240,240), header_color:Color=BLACK, border_color:Color=rgb(204,204,204)) -> Self:
        widths=[]
        for i,c in enumerate(columns):
            widths.append(c.width or (max([len(c.header),*(len(row[i]) if i<len(row) else 0 for row in rows)])*font_size*.52+2*padding))
        total=sum(widths);rh=font_size+2*padding
        self.rect(x,y,total,rh,RectOptions(fill=header_background));cx=x
        for i,c in enumerate(columns):
            tx=cx+(widths[i]/2 if c.align=="center" else widths[i]-padding if c.align=="right" else padding)
            self.text(c.header,tx,y+padding+font_size*.8,TextOptions(font_size,header_color,c.align,"bold"));cx+=widths[i]
        cy=y+rh
        for ri,row in enumerate(rows):
            self.rect(x,cy,total,rh,RectOptions(fill=rgb(249,249,249) if ri%2 else rgb(255,255,255)));cx=x
            for i,c in enumerate(columns):
                tx=cx+(widths[i]/2 if c.align=="center" else widths[i]-padding if c.align=="right" else padding)
                self.text(row[i] if i<len(row) else "",tx,cy+padding+font_size*.8,TextOptions(font_size,BLACK,c.align));cx+=widths[i]
            cy+=rh
        self.rect(x,y,total,rh*(len(rows)+1),RectOptions(stroke=border_color));return self
    def bytes(self) -> bytes: return _minpdf.build(self._doc)
    def save(self,path:str|Path) -> None: Path(path).write_bytes(self.bytes())

__all__ = ["PDF","TextOptions","RectOptions","ImageOptions","TableColumn","Color","rgb","A3","A4","A5","LETTER"]
