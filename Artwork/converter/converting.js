const fs = require('fs'),
      PNG = require('pngjs').PNG

function convert(inFile, outFile){
    fs.createReadStream(inFile)
      .pipe(new PNG())
      .on('parsed', function() {        
        let outData =  Buffer.alloc(this.height*this.width*2 + 4);
        let outPos = 0;

        outData[outPos++] = this.width&0xff;
        outData[outPos++] = (this.width>>8)&0xff;
        outData[outPos++] = this.height&0xff;
        outData[outPos++] = (this.height>>8)&0xff;  
        for (var y = 0; y < this.height; y++) {
            for (var x = 0; x < this.width; x++) {                
                var idx = (this.width * y + x) << 2;
    
                const r = this.data[idx+0]
                const g = this.data[idx+1]
                const b = this.data[idx+2]
                const word = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                outData[outPos++] = (word>>8)&0xff;
                outData[outPos++] = word&0xff;                
            }
        }

        let outStr = 'const uint8_t data = [';
        outData.forEach(val => {
            outStr += '0x'+val.toString(16) +', '
        });
        outStr += '0x00, 0x00];'
        fs.writeFile(outFile, outData,  "binary",function(err) { });
        //fs.writeFile(outFile+".h", outStr,  "utf8", function(err) { });
        this.pack().pipe(fs.createWriteStream('out.png'));
    });
}

convert ('MainMenu.png', 'MM.istl');
convert ('MainMenuDown.png', 'MMD.istl');
convert ('MainMenuDis.png', 'MMX.istl');