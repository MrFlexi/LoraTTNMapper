function lppDecode(bytes) {
    var sensor_types = {
        2: { 'size': 2, 'name': 'analog_in', 'signed': true, 'divisor': 100 },
        103: { 'size': 2, 'name': 'temperature', 'signed': true, 'divisor': 10 },
        104: { 'size': 1, 'name': 'humidity', 'signed': false, 'divisor': 2 },
        113: { 'size': 6, 'name': 'accelerometer', 'signed': true, 'divisor': 1000 },
        115: { 'size': 2, 'name': 'barometer', 'signed': false, 'divisor': 10 },
        116: { 'size': 2, 'name': 'voltage', 'signed': false, 'divisor': 100 },
        117: { 'size': 2, 'name': 'current', 'signed': false, 'divisor': 1000 },
        136: { 'size': 9, 'name': 'gps', 'signed': true, 'divisor': [10000, 10000, 100] },
    };

    function arrayToDecimal(stream, is_signed, divisor) {

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
            throw 'Sensor type error!: ' + s_type + ' Sno:' + _no + ' Pos:' + i;
        }

        var s_value = 0;
        var type = sensor_types[s_type];
        switch (s_type) {
            case 134:   // GyrO
                s_value = {
                    'x': arrayToDecimal(bytes.slice(i + 0, i + 2), type.signed, type.divisor),
                    'y': arrayToDecimal(bytes.slice(i + 2, i + 4), type.signed, type.divisor),
                    'z': arrayToDecimal(bytes.slice(i + 4, i + 6), type.signed, type.divisor)
                };
                break;

            case 136:   // GPS
                s_value = {
                    'latitude': arrayToDecimal(bytes.slice(i + 0, i + 3), type.signed, type.divisor[0]),
                    'longitude': arrayToDecimal(bytes.slice(i + 3, i + 6), type.signed, type.divisor[1]),
                    'altitude': arrayToDecimal(bytes.slice(i + 6, i + 9), type.signed, type.divisor[2])
                };
                break;

            default:    // All the rest
                s_value = arrayToDecimal(bytes.slice(i, i + type.size), type.signed, type.divisor);
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
        OUT.latitude = ((input.bytes[0] << 16) >>> 0) + ((input.bytes[1] << 8) >>> 0) + input.bytes[2];
        OUT.latitude = (OUT.latitude / 16777215.0 * 180) - 90;

        OUT.longitude = ((input.bytes[3] << 16) >>> 0) + ((input.bytes[4] << 8) >>> 0) + input.bytes[5];
        OUT.longitude = (OUT.longitude / 16777215.0 * 360) - 180;

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

let buffer = new Uint8Array([0x10, 0x02, 0x00, 0x01]);
console.log( buffer );

var ttn = { bytes:buffer,
            fPort: 2 };
console.log( ttn );

var a = decodeUplink( ttn );
console.log( a );
