function lppDecode(bytes) {
    var sensor_types = {
        2: { 'size': 2, 'name': 'analog_in', 'signed': true, 'divisor': 100 },
        103: { 'size': 2, 'name': 'temp', 'signed': true, 'divisor': 10 },        
        113: { 'size': 6, 'name': 'accelerometer', 'signed': true, 'divisor': 1000 },        
        136: { 'size': 9, 'name': 'gps', 'signed': true, 'divisor': [10000, 10000, 100] },
        201: { 'size': 12, 'name': 'pmu', 'signed': true, 'divisor': 100 },
    };

    function arrToDec(stream, is_signed, divisor) {

        var value = 0;
        for (var i = 0; i < stream.length; i++) {
            if (stream[i] > 0xFF)
                throw 'Byte value overflow!';
            value = (value << 8) | stream[i];
        }

        if (is_signed) {
            var edge = 1 << (stream.length) * 8; 
            var max = (edge - 1) >> 1;          
            value = (value > max) ? value - edge : value;
        }
        value /= divisor;
        return value;
    }

    var sensors = [];
    var i = 0;
    while (i < bytes.length) {

        var s_no = bytes[i++];
        var s_type = bytes[i++];
        if (typeof sensor_types[s_type] == 'undefined') {
            posi = i - 1;
            throw 'Type error!: ' + s_type + ' Sno:' + _no + ' Pos:' + i;
        }

        var s_value = 0;
        var type = sensor_types[s_type];
        switch (s_type) {
            case 136:   // GPS
                s_value = {
                    'lat': arrToDec(bytes.slice(i + 0, i + 3), type.signed, type.divisor[0]),
                    'lon': arrToDec(bytes.slice(i + 3, i + 6), type.signed, type.divisor[1]),
                    'altitude': arrToDec(bytes.slice(i + 6, i + 9), type.signed, type.divisor[2])
                };
                break;
                case 201: //PMU
                s_value = {
                    'bus_voltage': arrToDec(bytes.slice(i + 0, i + 2), type.signed, type.divisor),
                    'bus_current': arrToDec(bytes.slice(i + 2, i + 4), type.signed, type.divisor),
                    'bat_voltage': arrToDec(bytes.slice(i + 4, i + 6), type.signed, type.divisor),
                    'bat_charge_current': arrToDec(bytes.slice(i + 6, i + 8), type.signed, type.divisor),
                    'bat_discharge_current': arrToDec(bytes.slice(i + 8, i + 10), type.signed, type.divisor),
                    'bat_DeltamAh': arrToDec(bytes.slice(i + 10, i + 12), type.signed, type.divisor),
                };
               break;

            default:   
                s_value = arrToDec(bytes.slice(i, i + type.size), type.signed, type.divisor);
                break;
        }

        sensors.push({
            'channel': s_no,
            'type': s_type,
            'name': type.name,
            'value': s_value
        });
        i += type.size;
    }
    return sensors;
}

function decodeUplink(input) {
    var data = {};
    var warnings = [];
    var response = {};
    
    var  OUT = {};
    if (input.fPort === 1) {
        OUT.lat = ((input.bytes[0] << 16) >>> 0) + ((input.bytes[1] << 8) >>> 0) + input.bytes[2];
        OUT.lat = (OUT.lat / 16777215.0 * 180) - 90;

        OUT.lon = ((input.bytes[3] << 16) >>> 0) + ((input.bytes[4] << 8) >>> 0) + input.bytes[5];
        OUT.lon = (OUT.lon / 16777215.0 * 360) - 180;

        var altValue = ((input.bytes[6] << 8) >>> 0) + input.bytes[7];
        var sign = input.bytes[6] & (1 << 7);
        if (sign) {
            OUT.altitude = 0xFFFF0000 | altValue;
        }
        else {
            OUT.altitude = altValue;
        }

        OUT.hdop = input.bytes[8] / 10.0;

        return {
            data: OUT,
            warnings: warnings
        };
    }
    if (input.fPort === 2) {
        lppDecode(input.bytes, 1).forEach(function (field) {
            response[field['name'] + '_' + field['channel']] = field['value'];
        });
        return {
            data: response,
            warnings: warnings
        };
    }
}

// ab hier code um die Funktion zu testen

let buffer = new Uint8Array([0x10, 0xC9, 0x00, 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01]);
console.log( buffer );

var ttn = { bytes:buffer,
            fPort: 2 };
console.log( ttn );

var a = decodeUplink( ttn );
console.log( a );
