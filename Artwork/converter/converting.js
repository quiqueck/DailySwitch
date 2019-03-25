const fs = require('fs'),
      PNG = require('pngjs').PNG,
      SINGLE_PRESS=1, 
      DOUBLE_PRESS=0, 
      LONG_PRESS=2, 
      ON=1, 
      OFF=0, 
      ON_SECONDARY=2,
      SWITCH=0, 
      PAGE=1, 
      SELECT=2;

function convert(inFile, outFile, alpha){
    fs.createReadStream(inFile)
      .pipe(new PNG())
      .on('parsed', function() {        
        let sz = alpha ? 3 : 2;
        let outData =  Buffer.alloc(this.height*this.width*sz + 4);
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
                if (alpha) {
                    outData[outPos++] = this.data[idx+3]
                }            
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

function writeButtons(buttons, pages, outFile){
    let outData =  Buffer.alloc(buttons.length * 22 + 2 + pages.length * 2);
        let outPos = 0;

        outData[outPos++] = buttons.length&0xff; 
        outData[outPos++] = pages.length&0xff; 
        for (var i = 0; i < pages.length; i++) {
            for (let k=0; k<2; k++){
                let c = pages[i][k] ? pages[i].charCodeAt(k) : 32;  
                outData[outPos++] = c&0xff;
            }
        }      
        for (var i = 0; i < buttons.length; i++) {
            if (buttons[i].w) buttons[i].r = buttons[i].l + buttons[i].w;
            if (buttons[i].h) buttons[i].b = buttons[i].t + buttons[i].h;

            buttons[i].l -= 5;
            buttons[i].t -= 5;
            buttons[i].r += 5;
            buttons[i].b += 5;
            
            outData[outPos++] = buttons[i].l&0xff;
            outData[outPos++] = (buttons[i].l>>8)&0xff;

            outData[outPos++] = buttons[i].t&0xff;
            outData[outPos++] = (buttons[i].t>>8)&0xff;

            
            outData[outPos++] = buttons[i].r&0xff;
            outData[outPos++] = (buttons[i].r>>8)&0xff;

            
            outData[outPos++] = buttons[i].b&0xff;
            outData[outPos++] = (buttons[i].b>>8)&0xff;

            outData[outPos++] = buttons[i].type&0xff;

            outData[outPos++] = buttons[i].id&0xff;

            outData[outPos++] = buttons[i].state&0xff;

            if (buttons[i].altState === undefined) buttons[i].altState = buttons[i].state;
            outData[outPos++] = buttons[i].altState&0xff;

            outData[outPos++] = buttons[i].page&0xff;

            for (let k=0; k<8; k++){
                let c = buttons[i].name[k] ? buttons[i].name.charCodeAt(k) : 32;  
                outData[outPos++] = c&0xff;
            }
            outData[outPos++] = 0;
        }

        
        fs.writeFile(outFile, outData,  "binary",function(err) { });    
}

convert ('MainMenu.png', 'MM.IST', false);
convert ('MainMenuDown.png', 'MMD.IST', false);
convert ('MainMenuDis.png', 'MMX.IST', false);

convert ('Plan.png', 'PL.IST', false);
convert ('PlanDown.png', 'PLD.IST', false);
convert ('PlanDis.png', 'PLX.IST', false);

convert ('LightLevel.png', 'LL.IST', false);
convert ('LightLevelDown.png', 'LLD.IST', false);
convert ('LightLevelDis.png', 'LLX.IST', false);

let buttons = [
    {l:22, t:28, w:70, h:52, 
        type:SWITCH, id:1, state:ON_SECONDARY,
        name:"WOHNZ",
        page:0
    },
    {l:22, t:100, w:70, h:52, 
        type:SWITCH, id:1, state:ON,
        name:"WOHNZ",
        page:0
    },
    {l:22, t:171, w:70, h:52, 
        type:SWITCH, id:1, state:OFF,
        name:"WOHNZ",
        page:0
    },
    {l:114, t:28, w:70, h:52, 
        type:SWITCH, id:2, state:ON_SECONDARY,
        name:"ESSZ",
        page:0
    },
    {l:114, t:100, w:70, h:52, 
        type:SWITCH, id:2, state:ON,
        name:"ESSZ",
        page:0
    },
    {l:114, t:171, w:70, h:52, 
        type:SWITCH, id:2, state:OFF,
        name:"ESSZ",
        page:0
    },
    {l:206, t:28, w:70, h:52, 
        type:SWITCH, id:3, state:ON_SECONDARY,
        name:"KUCHE",
        page:0
    },
    {l:206, t:100, w:70, h:52, 
        type:SWITCH, id:3, state:ON,
        name:"KUCHE",
        page:0
    },
    {l:206, t:171, w:70, h:52, 
        type:SWITCH, id:3, state:OFF,
        name:"KUCHE",
        page:0
    },
    {l:317, t:28, w:70, h:52, 
        type:SWITCH, id:4, state:ON,
        altState:OFF, name:"SOUND",
        page:0
    },
    {l:317, t:100, w:70, h:52, 
        type:SWITCH, id:5, state:ON,
        altState:OFF, name:"SPECIAL",
        page:0
    },
    {l:317, t:100, w:70, h:52, 
        type:SWITCH, id:5, state:ON,
        altState:OFF, name:"SPECIAL",
        page:0
    },
    {l:418, t:223, w:48, h:48, 
        type:PAGE, id:1, state:ON,
        name:"BLUEPRNT",
        page:0
    },


    {l:418, t:178, w:48, h:48, 
        type:PAGE, id:0, state:ON,
        name:"FAVORITS",
        page:1
    },{l:35, t:64, w:48, h:48, 
        type:SELECT, id:6, state:ON,
        name:"LED_INN",
        page:1
    },{l:93, t:22, w:48, h:48, 
        type:SELECT, id:7, state:ON,
        name:"CANDL",
        page:1
    },{l:151, t:64, w:48, h:48, 
        type:SELECT, id:8, state:ON,
        name:"AVE_INN",
        page:1
    },{l:38, t:204, w:48, h:48, 
        type:SELECT, id:9, state:ON,
        name:"LED_OUT",
        page:1
    },{l:95, t:203, w:48, h:48, 
        type:SELECT, id:10, state:ON,
        name:"AVE_OUT",
        page:1
    },{l:182, t:252, w:48, h:48, 
        type:SELECT, id:11, state:ON,
        name:"LED_ESS",
        page:1
    },{l:203, t:188, w:48, h:48, 
        type:SELECT, id:12, state:ON,
        name:"ESSTIS",
        page:1
    },{l:256, t:131, w:48, h:48, 
        type:SELECT, id:13, state:ON,
        name:"KUC_LI",
        page:1
    },{l:325, t:131, w:48, h:48, 
        type:SELECT, id:14, state:ON,
        name:"KUC_RE",
        page:1
    }
];

writeButtons(buttons, ['MM', 'PL'], 'DEF.BTS');