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
      SELECT=2,
      SPECIAL_FUNCTION=3,
      SPECIAL_BUTTON_PROXIMITY_INC = 1,
      SPECIAL_BUTTON_PROXIMITY_DEC = 2,
      SPECIAL_BUTTON_SCREEN_OFF = 3,
      SPECIAL_BUTTON_UPDATE_WEATHER = 4,
      SPECIAL_BUTTON_SETTINGS = 5,
      SPECIAL_BUTTON_CALIBRATE_TOUCH = 6;

function convert(inFile, outFile, alpha){
    fs.createReadStream(inFile)
      .pipe(new PNG())
      .on('parsed', function() {      
        //console.log(inFile);  
        let sz = alpha ? 4 : 2;
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
                //if (outPos < 32) console.log(r, g, b);
                const word = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                outData[outPos++] = (word>>8)&0xff;
                outData[outPos++] = word&0xff;    
                if (alpha) {                    
                    outData[outPos++] = this.data[idx+3];
                    outData[outPos++] = this.data[idx+3];
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

function writeButtons(buttons, lightLevel, pages, outFile){
    let outData =  Buffer.alloc(buttons.length * 22 + 10 + pages.length * 2);
        let outPos = 0;

        outData[outPos++] = buttons.length&0xff; 
        outData[outPos++] = pages.length&0xff; 

        outData[outPos++] = lightLevel.x&0xff;
        outData[outPos++] = (lightLevel.x>>8)&0xff;
        outData[outPos++] = lightLevel.y&0xff;
        outData[outPos++] = (lightLevel.y>>8)&0xff;
        outData[outPos++] = lightLevel.w&0xff;
        outData[outPos++] = (lightLevel.w>>8)&0xff;
        outData[outPos++] = lightLevel.h&0xff;
        outData[outPos++] = (lightLevel.h>>8)&0xff;

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

convert ('LightLevel.png', 'LL.IST', true);
convert ('LightLevelDown.png', 'LLD.IST', true);
convert ('LightLevelDis.png', 'LLX.IST', true);

convert ('Settings.png', 'ST.IST', false);
convert ('SettingsDown.png', 'STD.IST', false);
convert ('SettingsDis.png', 'STX.IST', false);

let lightLevel = {x:(406-77)/2, y:(320-171)/2, w:77, h:171};

let buttons = [
    {l:419, t:14, w:54, h:78, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_SETTINGS, state:OFF, name:"SET",
        page:1
    },
    {l:45, t:42, w:48, h:48, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_PROXIMITY_INC, state:ON_SECONDARY,
        name:"PROXINC",
        page:0
    },
    {l:163, t:42, w:48, h:48, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_PROXIMITY_DEC, state:ON_SECONDARY,
        name:"PROXDEC",
        page:0
    },
    {l:51, t:147, w:100, h:48, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_UPDATE_WEATHER, state:ON_SECONDARY,
        name:"WEATHER",
        page:0
    },
    {l:51, t:259, w:100, h:48, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_CALIBRATE_TOUCH, state:ON_SECONDARY,
        name:"CALIB",
        page:0
    },
    {l:178, t:259, w:100, h:48, 
        type:SPECIAL_FUNCTION, id:SPECIAL_BUTTON_SCREEN_OFF, state:ON_SECONDARY,
        name:"SCROFF",
        page:0
    },


    {l:22, t:28, w:70, h:52, 
        type:SWITCH, id:1, state:ON_SECONDARY,
        name:"WOHNZ",
        page:1
    },
    {l:22, t:100, w:70, h:52, 
        type:SWITCH, id:1, state:ON,
        name:"WOHNZ",
        page:1
    },
    {l:22, t:171, w:70, h:52, 
        type:SWITCH, id:1, state:OFF,
        name:"WOHNZ",
        page:1
    },
    {l:114, t:28, w:70, h:52, 
        type:SWITCH, id:2, state:ON_SECONDARY,
        name:"ESSZ",
        page:1
    },
    {l:114, t:100, w:70, h:52, 
        type:SWITCH, id:2, state:ON,
        name:"ESSZ",
        page:1
    },
    {l:114, t:171, w:70, h:52, 
        type:SWITCH, id:2, state:OFF,
        name:"ESSZ",
        page:1
    },
    {l:206, t:28, w:70, h:52, 
        type:SWITCH, id:3, state:ON_SECONDARY,
        name:"KUCHE",
        page:1
    },
    {l:206, t:100, w:70, h:52, 
        type:SWITCH, id:3, state:ON,
        name:"KUCHE",
        page:1
    },
    {l:206, t:171, w:70, h:52, 
        type:SWITCH, id:3, state:OFF,
        name:"KUCHE",
        page:1
    },
    {l:317, t:28, w:70, h:52, 
        type:SWITCH, id:5, state:OFF,
        altState:ON, name:"SPECIAL",
        page:1
    },
    {l:317, t:100, w:70, h:52, 
        type:SWITCH, id:4, state:OFF,
        altState:ON, name:"SOUND",
        page:1
    },
    {l:418, t:223, w:48, h:48, 
        type:PAGE, id:1, state:ON,
        name:"BLUEPRNT",
        page:1
    },


    {l:418, t:178, w:48, h:48, 
        type:PAGE, id:0, state:ON,
        name:"FAVORITS",
        page:2
    },{l:35, t:64, w:48, h:48, 
        type:SELECT, id:6, state:ON,
        name:"LED_INN",
        page:2
    },{l:93, t:22, w:48, h:48, 
        type:SELECT, id:7, state:ON,
        name:"CANDL",
        page:2
    },{l:151, t:64, w:48, h:48, 
        type:SELECT, id:8, state:ON,
        name:"AVE_INN",
        page:2
    },{l:38, t:204, w:48, h:48, 
        type:SELECT, id:9, state:ON,
        name:"LED_OUT",
        page:2
    },{l:95, t:203, w:48, h:48, 
        type:SELECT, id:10, state:ON,
        name:"AVE_OUT",
        page:2
    },{l:182, t:252, w:48, h:48, 
        type:SELECT, id:11, state:ON,
        name:"LED_ESS",
        page:2
    },{l:203, t:188, w:48, h:48, 
        type:SELECT, id:12, state:ON,
        name:"ESSTIS",
        page:2
    },{l:256, t:131, w:48, h:48, 
        type:SELECT, id:13, state:ON,
        name:"KUC_RE",
        page:2
    },{l:325, t:131, w:48, h:48, 
        type:SELECT, id:14, state:ON,
        name:"KUC_LI",
        page:2
    },
    
    
    {l:lightLevel.x + 5, t:lightLevel.y + 5, w:lightLevel.w - 10, h:62-10, 
        type:SWITCH, id:0xff, state:ON_SECONDARY,
        name:"LL_FULL",
        page:0xF
    },
    {l:lightLevel.x + 5, t:lightLevel.y + 62 + 5, w:lightLevel.w - 10, h:49-10, 
        type:SWITCH, id:0xff, state:ON,
        name:"LL_MID",
        page:0xF
    },
    {l:lightLevel.x + 5, t:lightLevel.y + 109 + 5, w:lightLevel.w - 10, h:(lightLevel.h - 109)-10, 
        type:SWITCH, id:0xff, state:OFF,
        name:"LL_OFF",
        page:0xF
    }
];

writeButtons(buttons, lightLevel, ['ST', 'MM', 'PL'], 'DEF.BTS');

convert ('../weather-icons/weather-28.png', '01d.IST', true);
convert ('../weather-icons/weather-32.png', '01n.IST', true);
convert ('../weather-icons/weather-4.png',  '02d.IST', true);
convert ('../weather-icons/weather-11.png', '02n.IST', true);
convert ('../weather-icons/weather-0.png',  '03d.IST', true);
convert ('../weather-icons/weather-0.png',  '03n.IST', true);
convert ('../weather-icons/weather-33.png', '04d.IST', true);
convert ('../weather-icons/weather-33.png', '04n.IST', true);
convert ('../weather-icons/weather-36.png', '09d.IST', true);
convert ('../weather-icons/weather-36.png', '09n.IST', true);
convert ('../weather-icons/weather-10.png', '10d.IST', true);
convert ('../weather-icons/weather-20.png', '10n.IST', true);
convert ('../weather-icons/weather-7.png',  '11d.IST', true);
convert ('../weather-icons/weather-7.png',  '11n.IST', true);
convert ('../weather-icons/weather-16.png', '13d.IST', true);
convert ('../weather-icons/weather-19.png', '13n.IST', true);
convert ('../weather-icons/weather-27.png', '50d.IST', true);
convert ('../weather-icons/weather-27.png', '50n.IST', true);

convert ('../weather-icons/weather-35.png', '781d.IST', true);
convert ('../weather-icons/weather-35.png', '781n.IST', true);