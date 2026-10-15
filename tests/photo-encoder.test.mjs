import test from 'node:test';
import assert from 'node:assert/strict';
import fs from 'node:fs';
import vm from 'node:vm';
const source=fs.readFileSync(new URL('../web/index.html',import.meta.url),'utf8');
const code=source.slice(source.indexOf('function encodePhotoGif('),source.indexOf('let photoImage=null'));
const ctx=vm.createContext({Uint8Array});vm.runInContext(code,ctx);
test('fotos se codifican como GIF estático 64x64 y cada píxel conserva su índice de paleta',()=>{
 const pixels=Uint8Array.from({length:64*64*4},(_,i)=>(i*73+19)%256);
 const gif=ctx.encodePhotoGif(pixels);
 assert.equal(Buffer.from(gif.slice(0,6)).toString(),'GIF89a');
 assert.equal(gif[6],64);assert.equal(gif[8],64);assert.ok(gif.length<6000);
 let pos=13+768;assert.equal(gif[pos++],44);pos+=9;assert.equal(gif[pos++],8);
 let packed=[];while(gif[pos]){const n=gif[pos++];packed.push(...gif.slice(pos,pos+n));pos+=n}pos++;assert.equal(gif[pos],59);
 let bit=0,size=9,dict=[],next=258,previous=null,decoded=[];
 const reset=()=>{dict=Array.from({length:256},(_,i)=>[i]);next=258;size=9;previous=null};reset();
 while(bit+size<=packed.length*8){let n=0;for(let k=0;k<size;k++,bit++)n|=((packed[bit>>3]>>(bit&7))&1)<<k;
  if(n===256){reset();continue}if(n===257)break;
  const entry=dict[n]||(n===next&&previous?[...previous,previous[0]]:null);assert.ok(entry,'código LZW válido');decoded.push(...entry);
  if(previous){dict[next++]=[...previous,entry[0]];if(next===(1<<size)&&size<12)size++}previous=entry;
 }
 assert.equal(decoded.length,4096);
 for(let i=0;i<4096;i++){let k=i*4;assert.equal(decoded[i],(pixels[k]>>5)<<5|(pixels[k+1]>>5)<<2|(pixels[k+2]>>6))}
});
