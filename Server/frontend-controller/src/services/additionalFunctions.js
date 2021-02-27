export async function connectChangerCeiling(data) {
  const response = await fetch('/connectChangerCeiling', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function endConnectionCeiling(data) {
  const response = await fetch('/endConnectionCeiling', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function changeColorCeiling(data) {
  const response = await fetch('/changeColorCeiling', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function connectChangerCouch(data) {
  const response = await fetch('/connectChangerCouch', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function endConnectionCouch(data) {
  const response = await fetch('/endConnectionCouch', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function changeColorCouch(data) {
  const response = await fetch('/changeColorCouch', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export async function sendCommand(data) {
  const response = await fetch('/sendCommand', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export const lightOptions = [
  { value: '1colceiling', label: 'Ceiling Strip 1', color: '#0052CC' },
  { value: '2colceiling', label: 'Ceiling Strip 2', color: '#5243AA' },
  { value: 'bothceiling', label: 'Ceiling - Both', color: '#00B8D9' },
  { value: 'bothcouch', label: 'Couch', color: '#5243AA' },
  { value: 'all', label: 'All', color: '#00B8D9' },
];
