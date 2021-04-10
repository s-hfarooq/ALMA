export async function sendCommand(data) {
  const response = await fetch('/sendCommand', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export function getCommandString(color, option) {
  let newColStr = color.rgb.r + "-" + color.rgb.g + "-" + color.rgb.b;

  // Figure out what output string ending needs to be
  switch(option) {
    case "1colceiling":
      newColStr += "-1-1-0-";
      break;
    case "1colcouch":
      newColStr += "-1-2-0-";
      break;
    case "2colceiling":
      newColStr += "-2-1-0-";
      break;
    case "2colcouch":
      newColStr += "-2-2-0-";
      break;
    case "bothceiling":
      newColStr += "-0-0-0-";
      break;
    case "bothcouch":
      newColStr += "-0-2-0-";
      break;
    default:
      newColStr += "-0-0-0-";
  }

  return newColStr;
}

export const lightOptions = [
  { value: 'bothceiling', label: 'Ceiling - Both', color: '#00B8D9' },
];

export const animationOptions = [
  // { value: "0", label: "blinkLeds_chase2" },
  // { value: "1", label: "colorPalette" },
  // { value: "2", label: "blinkLeds_simple" },
  // { value: "3", label: "blinkLeds_chase" },
  // { value: "4", label: "cylon" },
  // { value: "5", label: "colorTemperature" },
  { value: "2", label: "meteorRain" },
  // { value: "7", label: "confetti" },
  // { value: "8", label: "fadeInFadeOut" },
  // { value: "9", label: "cylon2" },
  // { value: "10", label: "sparkle" },
  // { value: "11", label: "snowSparkle" },
  // { value: "12", label: "runningLights" },
  // { value: "13", label: "colorWipe" },
  // { value: "14", label: "rainbowCycle" },
  // { value: "15", label: "theaterChase" },
  { value: "1", label: "theaterChaseRainbow" },
  // { value: "17", label: "alternatingRainbow" },
  // { value: "18", label: "advancedAlternatingRainbow" },
  // { value: "19", label: "strobe" }
];
