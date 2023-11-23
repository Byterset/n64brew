
const fs = require('fs');
const path = require('path');

let output = '';
let inputs = [];
let definePrefix = '';
let lastCommand = '';

for (let i = 2; i < process.argv.length; ++i) {
    const arg = process.argv[i];
    if (lastCommand) {
        if (lastCommand == '-o') {
            output = arg;
        } else if (lastCommand == '-p') {
            definePrefix = arg;
        }
        lastCommand = '';
    } else if (arg[0] == '-') {
        lastCommand = arg;
    } else {
        inputs.push(arg);
    }
}

inputs.push('TOTAL_COUNT');

const invalidCharactersRegex = /[^\w\d_]+/gm;

function formatSoundName(soundFilename, index) {
    const extension = path.extname(soundFilename);
    const lastPart = path.basename(soundFilename, extension);
    const defineName = definePrefix + lastPart.replace(invalidCharactersRegex, '_').toUpperCase();
    return `#define ${defineName} ${index}`;
}

function formatFile(outputFilename, soundFilenames) {
    const defineName = outputFilename.replace(invalidCharactersRegex, '_').toUpperCase();
    let descSoundfiles = soundFilenames.slice();
    descSoundfiles.pop();
    return `/**
* @file ${path.basename(outputFilename)}
* @brief auto generated header containing the IDs 
* of audio files:
* ${descSoundfiles.join('\n* ')}
* for reference when using the audio player
*/
#ifndef ${defineName}
#define ${defineName}

${soundFilenames.map(formatSoundName).join('\n')}

#endif`
}

fs.writeFileSync(output, formatFile(output, inputs));