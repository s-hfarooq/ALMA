export async function changeColor(data) {
  const response = await fetch('/colorChanger', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response.json();
}

export async function connectC(data) {
  const response = await fetch('/connectC', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  //return await response;
}


export async function endConn(data) {
  const response = await fetch('/endConn', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}


export async function changeCol(data) {
  const response = await fetch('/changeCol', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}


export const lightOptions = [
  { value: 'off', label: 'Off', color: '#FF8B00' },
  { value: 'both', label: 'Both', color: '#00B8D9' },
  { value: 'col', label: 'Strip 1', color: '#0052CC' },
  { value: 'col2', label: 'Strip 2', color: '#5243AA' },
  { value: 'fade', label: 'Fade', color: '#FF5630' },
];
