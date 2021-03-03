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
      newColStr += "-0-1-0-";
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
  { value: 'all', label: 'All', color: '#00B8D9' },
  { value: '1colceiling', label: 'Ceiling Strip 1', color: '#0052CC' },
  { value: '2colceiling', label: 'Ceiling Strip 2', color: '#5243AA' },
  { value: 'bothceiling', label: 'Ceiling - Both', color: '#00B8D9' },
  { value: 'bothcouch', label: 'Couch', color: '#5243AA' },
];
