const $ = id => document.getElementById(id);
let graphData = null;
let currentRoute = null;
const colors = ["#df5270", "#2879e8", "#16a36a", "#f2a52b", "#8b5cf6", "#16a3bd"];

async function init() {
  try {
    const response = await fetch('/api/data');
    graphData = await response.json();
    if (!response.ok || graphData.error) throw new Error(graphData.error || 'Could not load city network.');
    populateLocations();
    renderGraph();
    renderMST();
    $('message').textContent = 'City network loaded. Road distances are stored in the C++ backend.';
    $('graphStatus').textContent = `${graphData.locations.length} locations · ${graphData.roads.length} roads`;
  } catch (err) {
    $('message').textContent = 'Could not connect to the C++ backend. Run run.bat and refresh this page.';
    $('graphStatus').textContent = 'Backend connection failed';
  }
}
function populateLocations() {
  const options = graphData.locations.map((name, i) => `<option value="${i}">${name}</option>`).join('');
  $('source').innerHTML = options;
  $('destination').innerHTML = options;
  $('source').value = '0';
  $('destination').value = String(graphData.locations.length - 1);
  $('locationList').innerHTML = graphData.locations.map((name, i) => `<div class="location-item"><i style="background:${colors[i % colors.length]}"></i>${name}</div>`).join('');
}
function coords(n) {
  const points = [
    {x:380,y:55},{x:180,y:145},{x:560,y:145},{x:125,y:335},{x:390,y:330},{x:635,y:400}
  ];
  if (n <= points.length) return points.slice(0,n);
  return Array.from({length:n},(_,i)=>({x:380+260*Math.cos(i*2*Math.PI/n-Math.PI/2),y:240+170*Math.sin(i*2*Math.PI/n-Math.PI/2)}));
}
function key(u,v){return u<v?`${u}-${v}`:`${v}-${u}`;}
function renderGraph() {
  if (!graphData) return;
  const pts = coords(graphData.locations.length);
  const routeEdges = new Set();
  if (currentRoute && currentRoute.path) for(let i=1;i<currentRoute.path.length;i++) routeEdges.add(key(currentRoute.path[i-1].id,currentRoute.path[i].id));
  let svg = '';
  graphData.roads.forEach(e => {
    const a=pts[e.u],b=pts[e.v],active=routeEdges.has(key(e.u,e.v));
    const dx=b.x-a.x,dy=b.y-a.y,len=Math.sqrt(dx*dx+dy*dy)||1;
    const lx=(a.x+b.x)/2-dy/len*13,ly=(a.y+b.y)/2+dx/len*13;
    svg += `<line class="road-line ${active?'active':''}" x1="${a.x}" y1="${a.y}" x2="${b.x}" y2="${b.y}"/>`;
    svg += `<text class="road-label" x="${lx}" y="${ly}">${e.w}</text>`;
  });
  pts.forEach((p,i)=>{
    const onRoute=currentRoute&&currentRoute.path&&currentRoute.path.some(x=>x.id===i);
    svg += `<circle class="node ${onRoute?'selected':''}" cx="${p.x}" cy="${p.y}" r="17"/>`;
    svg += `<text class="node-label" x="${p.x}" y="${p.y+34}">${graphData.locations[i]}</text>`;
  });
  $('graph').innerHTML=svg;
}
function renderMST() {
  for(const alg of ['prim','kruskal']) {
    const result=graphData[alg];
    $(`${alg}Cost`).textContent=result.cost;
    $(`${alg}Edges`).innerHTML=result.edges.map(e=>`<div class="edge-row"><span>${e.from} ↔ ${e.to}</span><span>${e.w} km</span></div>`).join('');
  }
  $('mstTotal').textContent=graphData.prim.cost;
}
async function findRoute() {
  if(!graphData)return;
  const source=Number($('source').value), destination=Number($('destination').value);
  if(source===destination){$('message').textContent='Please choose two different locations.';return;}
  $('findRoute').disabled=true;$('findRoute').textContent='Calculating route…';
  try {
    const response=await fetch('/api/route',{method:'POST',headers:{'Content-Type':'text/plain'},body:`${source} ${destination}`});
    const result=await response.json();if(!response.ok||result.error)throw new Error(result.error||'Unable to calculate route.');
    currentRoute=result;
    $('routeDistance').innerHTML=`${result.distance} <small>km</small>`;
    $('pathVisual').innerHTML=result.path.map((p,i)=>`${i?'<span class="path-arrow">→</span>':''}<span class="path-node">${p.name}</span>`).join('');
    $('routeDetails').textContent=`Best route from ${result.source} to ${result.destination}: ${result.path.map(p=>p.name).join(' → ')}. Total distance: ${result.distance} km. Calculated by Dijkstra's algorithm in the C++ backend.`;
    $('message').textContent='Optimal route calculated successfully.';
    $('graphStatus').textContent=`Shortest route highlighted · ${result.distance} km`;
    renderGraph();
  } catch(err) { $('message').textContent=err.message; }
  finally {$('findRoute').disabled=false;$('findRoute').textContent='⌁ Find optimal route';}
}
$('findRoute').addEventListener('click',findRoute);
$('reset').addEventListener('click',()=>{if(!graphData)return;$('source').value='0';$('destination').value=String(graphData.locations.length-1);currentRoute=null;$('routeDistance').innerHTML='— <small>km</small>';$('pathVisual').innerHTML='<div class="placeholder">Choose your source and destination, then click <b>Find optimal route</b>.</div>';$('routeDetails').textContent='Your route steps will appear here.';$('message').textContent='Selection reset. Choose your locations.';$('graphStatus').textContent=`${graphData.locations.length} locations · ${graphData.roads.length} roads` ;renderGraph();});
init();
