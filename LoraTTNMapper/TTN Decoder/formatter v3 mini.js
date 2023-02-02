function lppDecode(e){
    var i={0:{size:1,name:"digital_in",signed:!1,divisor:1},
           1:{size:1,name:"digital_out",signed:!1,divisor:1},
           2:{size:2,name:"analog_in",signed:!0,divisor:100},
           100:{size:4,name:"generic",signed:!1,divisor:1},
           103:{size:2,name:"temperature",signed:!0,divisor:10},
           104:{size:1,name:"humidity",signed:!1,divisor:2},
           113:{size:6,name:"accelerometer",signed:!0,divisor:1e3},
           115:{size:2,name:"barometer",signed:!1,divisor:10},
           116:{size:2,name:"voltage",signed:!1,divisor:100},
           117:{size:2,name:"current",signed:!1,divisor:1e3},
           120:{size:1,name:"percentage",signed:!1,divisor:1},
           121:{size:2,name:"altitude",signed:!0,divisor:1},
           128:{size:2,name:"power",signed:!1,divisor:1},
           130:{size:4,name:"distance",signed:!1,divisor:1e3},
           131:{size:4,name:"energy",signed:!1,divisor:1e3},
           134:{size:6,name:"gyrometer",signed:!0,divisor:100},
           136:{size:9,name:"gps",signed:!0,divisor:[1e4,1e4,100]},
           142:{size:1,name:"switch",signed:!1,divisor:1}};

           function s(e,i,s){for(var n=0,d=0;d<e.length;d++){
               if(e[d]>255)throw"Byte value overflow!";n=n<<8|e[d]}
               if(i){var r=1<<8*e.length;n=n>r-1>>1?n-r:n}
               return n/=s}
               for(var n=[],d=0;d<e.length;){var r=e[d++],t=e[d++];if(void 0===i[t])throw posi=d-1,"Sensor type error!: "+t+" Sno:"+_no+" Pos:"+d;var a=0,o=i[t];
    switch(t){
        case 134:a={x:s(e.slice(d+0,d+2),o.signed,o.divisor),y:s(e.slice(d+2,d+4),o.signed,o.divisor),z:s(e.slice(d+4,d+6),o.signed,o.divisor)};break;
        case 136:a={latitude:s(e.slice(d+0,d+3),o.signed,o.divisor[0]),longitude:s(e.slice(d+3,d+6),o.signed,o.divisor[1]),altitude:s(e.slice(d+6,d+9),o.signed,o.divisor[2])};break;

        default:a=s(e.slice(d,d+o.size),o.signed,o.divisor)}n.push({channel:r,type:t,name:o.name,value:a}),d+=o.size}return n}
        
function decodeUplink(e){
 var i={};

if(1===e.fPort){OUT.latitude=(e.bytes[0]<<16>>>0)+(e.bytes[1]<<8>>>0)+e.bytes[2],OUT.latitude=OUT.latitude/16777215*180-90,OUT.longitude=(e.bytes[3]<<16>>>0)+(e.bytes[4]<<8>>>0)+e.bytes[5],OUT.longitude=OUT.longitude/16777215*360-180;
    var s=(e.bytes[6]<<8>>>0)+e.bytes[7],n=128&e.bytes[6];
    return OUT.altitude=n?4294901760|s:s,OUT.hdop=e.bytes[8]/10,{data:OUT,warnings:i}
}

if(2===e.fPort)
    return lppDecode(e.bytes,1).forEach(function(e){response[e.name+"_"+e.channel]=e.value}),{data:OUT,warnings:i}
}