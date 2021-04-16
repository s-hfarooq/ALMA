export async function sendCommand(data) {
  const response = await fetch("/sendCommand", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ color: data })
  });
  return await response;
}

export function getCommandString(color, option) {
  let recieverUID = "";
  switch (option) {
    case "ceiling":
      recieverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID
      break;
    // case "couch":
    //   newColStr += "-0-2-0-";
    //   break;
    default:
      recieverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID
  }

  var returnVal = {
      "senderUID": "10000000",
      "recieverUID": recieverUID,
      "functionID": "-1",
      "data": [
          color.rgb.r,
          color.rgb.g,
          color.rgb.b,
      ],
  };

  return JSON.stringify(returnVal);
}

export function getAnimationString(animationNum) {
    let recieverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID

    var returnVal = {
        "senderUID": "10000000",
        "recieverUID": recieverUID,
        "functionID": animationNum,
        "data": [ ],
    };

    return JSON.stringify(returnVal);
}

export const lightOptions = [
  // { value: "all", label: "All", color: "#00B8D9" },
  { value: "ceiling", label: "Ceiling", color: "#00B8D9" },
  // { value: "couch", label: "Couch", color: "#5243AA" }
];

export const animationOptions = [
  { value: "0", label: "blinkLeds_chase2" },
  { value: "1", label: "colorPalette" },
  { value: "2", label: "blinkLeds_simple" },
  { value: "3", label: "blinkLeds_chase" },
  { value: "4", label: "cylon" },
  { value: "5", label: "colorTemperature" },
  { value: "6", label: "meteorRain" },
  { value: "7", label: "confetti" },
  { value: "8", label: "fadeInFadeOut" },
  { value: "9", label: "cylon2" },
  { value: "10", label: "sparkle" },
  { value: "11", label: "snowSparkle" },
  { value: "12", label: "runningLights" },
  { value: "13", label: "colorWipe" },
  { value: "14", label: "rainbowCycle" },
  { value: "15", label: "theaterChase" },
  { value: "16", label: "theaterChaseRainbow" },
  { value: "17", label: "alternatingRainbow" },
  { value: "18", label: "advancedAlternatingRainbow" },
  { value: "19", label: "strobe" }
];
