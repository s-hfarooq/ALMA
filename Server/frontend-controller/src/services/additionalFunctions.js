export async function sendCommand(data) {
  const response = await fetch('/sendCommand', {
    method: 'POST',
    headers: {'Content-Type': 'application/json'},
    body: JSON.stringify({ color: data })
  })
  return await response;
}

export const lightOptions = [
  { value: 'all', label: 'All', color: '#00B8D9' },
  { value: '1colceiling', label: 'Ceiling Strip 1', color: '#0052CC' },
  { value: '2colceiling', label: 'Ceiling Strip 2', color: '#5243AA' },
  { value: 'bothceiling', label: 'Ceiling - Both', color: '#00B8D9' },
  { value: 'bothcouch', label: 'Couch', color: '#5243AA' },
];
