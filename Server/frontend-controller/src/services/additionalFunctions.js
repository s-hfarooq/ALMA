// Sends command to backend API
export async function sendCommand(data) {
  const response = await fetch("/sendCommand", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ color: data })
  });
  return await response;
}

// Creates command JSON string based on strip selected and current device ID
export function getCommandString(color, option, isFade, fadeSpeed) {
    let receiverUID = "", functionID = "", data = [];

    // Only couch can run fade command
    if(isFade == 1 && option == "ceiling")
        return;

    switch (option) {
        case "ceiling":
            receiverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID
            functionID = "-1";
            break;
        case "couch":
            receiverUID = "10200FFF"; // 102 = type 5050 LED controller, 00 = living room, FFF = any ID
            functionID = (isFade == 1) ? "3" : "0";
            break;
        case "hassan":
            receiverUID = "10203FFF"; // 102 = type 5050 LED controller, 03 = Hassan's room, FFF = any ID
            functionID = (isFade == 1) ? "3" : "0";
            break;
        default:
            receiverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID
    }

    if(isFade == 0)
        data = [color.rgb.r, color.rgb.g, color.rgb.b];
    else
        data = [fadeSpeed]

    var returnVal = {
        "senderUID": "10000000",
        "receiverUID": receiverUID,
        "functionID": functionID,
        "data": data,
    };

    return JSON.stringify(returnVal);
}

// Creates animation JSON string to send to backend API
export function getAnimationString(animationNum) {
    let receiverUID = "101FFFFF"; // 101 = type Holonyak, FF = any location, FFF = any ID

    var returnVal = {
        "senderUID": "10000000",
        "receiverUID": receiverUID,
        "functionID": animationNum,
        "data": [ ],
    };

    return JSON.stringify(returnVal);
}

// Available light strip options to display
export const lightOptions = [
    // { value: "all", label: "All", color: "#00B8D9" },
    { value: "ceiling", label: "Ceiling", color: "#00B8D9" },
    { value: "couch", label: "Couch", color: "#5243AA" },
    { value: "hassan", label: "Hassan's Room", color: "#5243AA" }
];

// Available animations to display
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
    { value: "19", label: "strobe" },
    { value: "20", label: "rave" }
];
