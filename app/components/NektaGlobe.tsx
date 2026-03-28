'use client'

import { useEffect, useRef, useCallback, useState } from 'react'

const LAND_B64 = "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"
const MAP_W = 720, MAP_H = 360

async function buildLandMap(): Promise<Uint8Array> {
  const bin = atob(LAND_B64)
  const bytes = new Uint8Array(bin.length)
  for (let i = 0; i < bin.length; i++) bytes[i] = bin.charCodeAt(i)
  const ds = new DecompressionStream('deflate')
  const writer = ds.writable.getWriter()
  writer.write(bytes); writer.close()
  const chunks: Uint8Array[] = []
  const reader = ds.readable.getReader()
  while (true) { const { done, value } = await reader.read(); if (done) break; chunks.push(value) }
  const total = chunks.reduce((s, c) => s + c.length, 0)
  const result = new Uint8Array(total)
  let off = 0; for (const c of chunks) { result.set(c, off); off += c.length }
  return result
}

function isLand(map: Uint8Array, lat: number, lon: number): boolean {
  while (lon > 180) lon -= 360; while (lon < -180) lon += 360
  const col = Math.min(MAP_W-1, Math.max(0, Math.floor((lon+180)/360*MAP_W)))
  const row = Math.min(MAP_H-1, Math.max(0, Math.floor((90-lat)/180*MAP_H)))
  return map[row*MAP_W+col] === 1
}

interface Hotspot { name: string; desc: string; lat: number; lon: number; tier: 0|1|2 }
interface EventData { id: string; title: string; startTime: string; venue?: { name: string }; artists?: { name: string }[] }

// Precise GPS coordinates verified
const HOTSPOTS: Hotspot[] = [
  { name: 'London',      desc: 'Fabric · Egg · Printworks',         lat: 51.51,  lon: -0.13,   tier: 2 },
  { name: 'Berlin',      desc: 'Berghain · Tresor · Watergate',     lat: 52.52,  lon: 13.40,   tier: 2 },
  { name: 'Ibiza',       desc: 'DC10 · Pacha · Amnesia',            lat: 38.91,  lon: 1.43,    tier: 2 },
  { name: 'Amsterdam',   desc: 'Shelter · Awakenings · ADE',        lat: 52.37,  lon: 4.90,    tier: 1 },
  { name: 'Paris',       desc: 'Rex Club · Concrete · Batofar',     lat: 48.86,  lon: 2.35,    tier: 1 },
  { name: 'Bristol',     desc: 'Motion · Marble Factory · Thekla',  lat: 51.45,  lon: -2.59,   tier: 1 },
  { name: 'Barcelona',   desc: 'Razzmatazz · Input · Apolo',        lat: 41.39,  lon: 2.15,    tier: 1 },
  { name: 'Detroit',     desc: 'Movement Festival · CODA',          lat: 42.33,  lon: -83.05,  tier: 1 },
  { name: 'Chicago',     desc: 'Smart Bar · Spy Bar',               lat: 41.88,  lon: -87.63,  tier: 1 },
  { name: 'New York',    desc: 'Output · House of Yes · Nowadays',  lat: 40.71,  lon: -74.01,  tier: 1 },
  { name: 'Rio',         desc: 'Fosfobox · Green Valley',           lat: -22.91, lon: -43.17,  tier: 1 },
  { name: 'Sao Paulo',   desc: 'D-Edge · Lov · The Week',           lat: -23.55, lon: -46.63,  tier: 1 },
  { name: 'Bangkok',     desc: 'De Commune · Onyx · Beam',          lat: 13.75,  lon: 100.50,  tier: 1 },
  { name: 'Koh Phangan', desc: 'Full Moon Party · Shiva Moon',      lat: 9.73,   lon: 100.01,  tier: 1 },
  { name: 'Melbourne',   desc: 'Revolver · Brown Alley · 161',      lat: -37.81, lon: 144.96,  tier: 1 },
  { name: 'Tokyo',       desc: 'Womb · Contact · Dommune',          lat: 35.69,  lon: 139.69,  tier: 1 },
  { name: 'Bali',        desc: 'Potato Head · La Favela',           lat: -8.34,  lon: 115.09,  tier: 0 },
  { name: 'Cape Town',   desc: 'Shimmy Beach · Cause Effect',       lat: -33.93, lon: 18.42,   tier: 0 },
  { name: 'Prague',      desc: 'Ankali · Cross Club',               lat: 50.08,  lon: 14.44,   tier: 0 },
  { name: 'Zurich',      desc: 'Hive · Zukunft · Supermarket',      lat: 47.38,  lon: 8.54,    tier: 0 },
  { name: 'Dubai',       desc: 'White Dubai · Soho Garden',         lat: 25.20,  lon: 55.27,   tier: 0 },
  { name: 'Seoul',       desc: 'Cakeshop · Contra · Boogie Nights', lat: 37.57,  lon: 126.98,  tier: 0 },
  { name: 'LA',          desc: 'Exchange · Catch One · EDC',        lat: 34.05,  lon: -118.24, tier: 0 },
  { name: 'Toronto',     desc: 'Coda · Rebel · Velvet Underground', lat: 43.65,  lon: -79.38,  tier: 0 },
  { name: 'Bogota',      desc: 'Baum · Video Club · Armando',       lat: 4.71,   lon: -74.07,  tier: 0 },
]

function buildDots(map: Uint8Array) {
  const dots: { lat: number; lon: number }[] = []
  const STEP = 1.1
  for (let lat = -90+STEP/2; lat < 90; lat += STEP) {
    const num = Math.max(1, Math.round(360*Math.cos(lat*Math.PI/180)/STEP))
    for (let i = 0; i < num; i++) {
      const lon = -180 + (i+0.5)*360/num
      if (isLand(map, lat, lon)) dots.push({ lat, lon })
    }
  }
  return dots
}

const SIZE = 580, GLOB_R = 270, MAX_THETA = 0.48

// Project lat/lon to screen xy — returns null if on back of globe
function makeProjector(rotLon: number, rotTheta: number, scale: number) {
  const CX = SIZE/2, CY = SIZE/2, R = GLOB_R*scale
  return function project(lat: number, lon: number) {
    const dLon = ((lon - rotLon + 540) % 360) - 180
    if (Math.abs(dLon) >= 90) return null
    const la = lat*Math.PI/180, lo = dLon*Math.PI/180
    const th = rotTheta
    const x = Math.cos(la)*Math.sin(lo)
    const y = Math.sin(la), z = Math.cos(la)*Math.cos(lo)
    const y2 = y*Math.cos(th)+z*Math.sin(th)
    const z2 = -y*Math.sin(th)+z*Math.cos(th)
    if (z2 < 0.01) return null
    return { x: CX+x*R, y: CY-y2*R, z: z2 }
  }
}

function findClickedHotspot(mx: number, my: number, rotLon: number, rotTheta: number, scale: number): Hotspot | null {
  const project = makeProjector(rotLon, rotTheta, scale)
  let best: Hotspot | null = null, bestD = 9999
  for (const h of HOTSPOTS) {
    const p = project(h.lat, h.lon)
    if (!p) continue
    const d = Math.hypot(mx - p.x, my - p.y)
    // Hit radius proportional to tier and depth
    const hitR = (h.tier === 2 ? 16 : h.tier === 1 ? 13 : 10) * scale * p.z
    if (d < hitR && d < bestD) { bestD = d; best = h }
  }
  return best
}

export function NektaGlobe() {
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const rotLonRef = useRef(10), targetLonRef = useRef(10)
  const rotThetaRef = useRef(0), targetThetaRef = useRef(0)
  const scaleRef = useRef(1), targetScaleRef = useRef(1)
  const pointerDownRef = useRef(false)
  const lastXRef = useRef(0), lastYRef = useRef(0)
  const pinchRef = useRef(0), rafRef = useRef(0)
  const didDragRef = useRef(false)
  const dotsRef = useRef<{ lat: number; lon: number }[]>([])
  const pulseRef = useRef(0)
  const spinningRef = useRef(true)

  const [activeHotspot, setActiveHotspot] = useState<Hotspot | null>(null)
  const [spinning, setSpinning] = useState(true)
  const [events, setEvents] = useState<Record<string, EventData[]>>({})
  const [visible, setVisible] = useState(false)
  const [loading, setLoading] = useState(true)

  useEffect(() => { spinningRef.current = spinning }, [spinning])

  const flyTo = useCallback((h: Hotspot) => {
    targetLonRef.current = h.lon
    targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, -(h.lat*Math.PI/180)*0.3))
    setSpinning(false)
    setActiveHotspot(h)
  }, [])

  const dismiss = useCallback(() => {
    setActiveHotspot(null)
    setSpinning(true)
  }, [])

  useEffect(() => {
    fetch('/api/events').then(r => r.json()).then(setEvents).catch(() => {})
  }, [])

  // Spacebar + Escape
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.target !== document.body && e.target !== document.documentElement) return
      if (e.code === 'Space') { e.preventDefault(); setSpinning(s => !s) }
      if (e.code === 'Escape') dismiss()
    }
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [dismiss])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const ctx = canvas.getContext('2d')!
    canvas.width = SIZE; canvas.height = SIZE

    buildLandMap().then(map => {
      dotsRef.current = buildDots(map)
      setLoading(false)
      setTimeout(() => setVisible(true), 100)
    })

    function draw() {
      if (spinningRef.current && !pointerDownRef.current) targetLonRef.current += 0.1
      rotLonRef.current += (targetLonRef.current - rotLonRef.current) * 0.06
      rotThetaRef.current += (targetThetaRef.current - rotThetaRef.current) * 0.06
      scaleRef.current += (targetScaleRef.current - scaleRef.current) * 0.08
      pulseRef.current += 0.04

      const R = GLOB_R * scaleRef.current
      const CX = SIZE/2, CY = SIZE/2
      const project = makeProjector(rotLonRef.current, rotThetaRef.current, scaleRef.current)

      ctx.clearRect(0, 0, SIZE, SIZE)

      // Sphere bg
      const bg = ctx.createRadialGradient(CX-40, CY-50, 10, CX, CY, R)
      bg.addColorStop(0, '#180604'); bg.addColorStop(0.65, '#0c0202'); bg.addColorStop(1, '#040101')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R, SIZE/2-1), 0, Math.PI*2)
      ctx.fillStyle = bg; ctx.fill()

      ctx.save()
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R-1, SIZE/2-2), 0, Math.PI*2); ctx.clip()

      const dotR = Math.max(0.9, 1.5*scaleRef.current)

      // Land dots
      for (const dot of dotsRef.current) {
        const p = project(dot.lat, dot.lon)
        if (!p) continue
        ctx.beginPath(); ctx.arc(p.x, p.y, dotR*(0.55+p.z*0.45), 0, Math.PI*2)
        ctx.fillStyle = `rgba(255,55,10,${Math.min(1, 0.5+p.z*0.45).toFixed(2)})`
        ctx.fill()
      }

      // Hotspot dots — drawn ON TOP of land dots
      for (const h of HOTSPOTS) {
        const p = project(h.lat, h.lon)
        if (!p) continue
        const pulse = Math.sin(pulseRef.current + h.lat*0.3) * 0.5 + 0.5
        const ts = h.tier===2 ? 1 : h.tier===1 ? 0.75 : 0.55

        // Outer ring
        const ringR = (7+pulse*9)*ts*scaleRef.current*p.z
        ctx.beginPath(); ctx.arc(p.x, p.y, ringR, 0, Math.PI*2)
        ctx.strokeStyle = `rgba(255,140,30,${((0.28-pulse*0.22)*p.z).toFixed(2)})`
        ctx.lineWidth = 0.9; ctx.stroke()

        // Glow halo
        const glowR = (5+pulse*4)*ts*scaleRef.current*p.z
        const grd = ctx.createRadialGradient(p.x, p.y, 0, p.x, p.y, glowR)
        grd.addColorStop(0, `rgba(255,210,90,${(0.95*p.z).toFixed(2)})`)
        grd.addColorStop(0.45, `rgba(255,110,15,${(0.5*p.z).toFixed(2)})`)
        grd.addColorStop(1, 'rgba(255,65,10,0)')
        ctx.beginPath(); ctx.arc(p.x, p.y, glowR, 0, Math.PI*2)
        ctx.fillStyle = grd; ctx.fill()

        // Bright core
        const coreR = (2+pulse*0.7)*ts*scaleRef.current*p.z
        ctx.beginPath(); ctx.arc(p.x, p.y, coreR, 0, Math.PI*2)
        ctx.fillStyle = `rgba(255,245,190,${Math.min(1, p.z*1.15).toFixed(2)})`
        ctx.fill()
      }

      ctx.restore()

      // Limb darkening
      const limb = ctx.createRadialGradient(CX, CY, R*0.5, CX, CY, R)
      limb.addColorStop(0, 'rgba(0,0,0,0)'); limb.addColorStop(1, 'rgba(0,0,0,0.88)')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R, SIZE/2-1), 0, Math.PI*2)
      ctx.fillStyle = limb; ctx.fill()

      // Atmosphere rim
      const rim = ctx.createRadialGradient(CX, CY, R*0.88, CX, CY, R*1.06)
      rim.addColorStop(0, 'rgba(255,65,32,0)'); rim.addColorStop(1, 'rgba(255,65,32,0.13)')
      ctx.beginPath(); ctx.arc(CX, CY, Math.min(R*1.06, SIZE/2), 0, Math.PI*2)
      ctx.fillStyle = rim; ctx.fill()

      rafRef.current = requestAnimationFrame(draw)
    }

    rafRef.current = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(rafRef.current)
  }, [])

  // Click handler — finds closest hotspot to click point
  const onCanvasClick = useCallback((e: React.MouseEvent<HTMLCanvasElement>) => {
    if (didDragRef.current) return
    const rect = (e.target as HTMLCanvasElement).getBoundingClientRect()
    const mx = (e.clientX - rect.left) * (SIZE / rect.width)
    const my = (e.clientY - rect.top) * (SIZE / rect.height)
    const hit = findClickedHotspot(mx, my, rotLonRef.current, rotThetaRef.current, scaleRef.current)
    if (hit) flyTo(hit)
    else dismiss()
  }, [flyTo, dismiss])

  const onPointerDown = useCallback((e: React.PointerEvent) => {
    pointerDownRef.current = true; didDragRef.current = false
    lastXRef.current = e.clientX; lastYRef.current = e.clientY
    ;(e.target as HTMLElement).setPointerCapture(e.pointerId)
  }, [])
  const onPointerUp = useCallback(() => { pointerDownRef.current = false }, [])
  const onPointerMove = useCallback((e: React.PointerEvent) => {
    if (!pointerDownRef.current) return
    const dx = e.clientX-lastXRef.current, dy = e.clientY-lastYRef.current
    if (Math.abs(dx)>3||Math.abs(dy)>3) didDragRef.current = true
    const sp = 0.4/scaleRef.current
    targetLonRef.current -= dx*sp
    targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, targetThetaRef.current-dy*sp*0.008))
    lastXRef.current = e.clientX; lastYRef.current = e.clientY
  }, [])
  const onWheel = useCallback((e: React.WheelEvent) => {
    e.preventDefault()
    if (e.ctrlKey||e.metaKey) {
      targetScaleRef.current = Math.max(0.5, Math.min(4, targetScaleRef.current*(1-e.deltaY*0.008)))
    } else if (Math.abs(e.deltaX)>Math.abs(e.deltaY)) {
      targetLonRef.current -= e.deltaX*0.3
    } else {
      targetThetaRef.current = Math.max(-MAX_THETA, Math.min(MAX_THETA, targetThetaRef.current-e.deltaY*0.002))
    }
  }, [])
  const lastTouchRef = useRef<{x:number,y:number}|null>(null)
  const onTouchStart = useCallback((e: React.TouchEvent) => {
    e.preventDefault(); didDragRef.current = false
    if (e.touches.length===1){pointerDownRef.current=true;lastTouchRef.current={x:e.touches[0].clientX,y:e.touches[0].clientY}}
    if (e.touches.length===2){pointerDownRef.current=false;pinchRef.current=Math.hypot(e.touches[1].clientX-e.touches[0].clientX,e.touches[1].clientY-e.touches[0].clientY)}
  }, [])
  const onTouchMove = useCallback((e: React.TouchEvent) => {
    e.preventDefault()
    if (e.touches.length===1&&lastTouchRef.current&&pointerDownRef.current){
      const dx=e.touches[0].clientX-lastTouchRef.current.x, dy=e.touches[0].clientY-lastTouchRef.current.y
      if (Math.abs(dx)>3||Math.abs(dy)>3) didDragRef.current=true
      const sp=0.4/scaleRef.current
      targetLonRef.current-=dx*sp
      targetThetaRef.current=Math.max(-MAX_THETA,Math.min(MAX_THETA,targetThetaRef.current-dy*sp*0.008))
      lastTouchRef.current={x:e.touches[0].clientX,y:e.touches[0].clientY}
    }
    if (e.touches.length===2&&pinchRef.current>0){
      const nd=Math.hypot(e.touches[1].clientX-e.touches[0].clientX,e.touches[1].clientY-e.touches[0].clientY)
      targetScaleRef.current=Math.max(0.5,Math.min(4,targetScaleRef.current*(nd/pinchRef.current)))
      pinchRef.current=nd
    }
  }, [])
  const onTouchEnd = useCallback((e: React.TouchEvent) => {
    if (!didDragRef.current&&e.changedTouches.length===1){
      const rect=canvasRef.current!.getBoundingClientRect()
      const mx=(e.changedTouches[0].clientX-rect.left)*(SIZE/rect.width)
      const my=(e.changedTouches[0].clientY-rect.top)*(SIZE/rect.height)
      const hit=findClickedHotspot(mx,my,rotLonRef.current,rotThetaRef.current,scaleRef.current)
      if (hit) flyTo(hit); else dismiss()
    }
    pointerDownRef.current=false; lastTouchRef.current=null; pinchRef.current=0
  }, [flyTo, dismiss])

  const cityEvents = activeHotspot ? (events[activeHotspot.name] ?? []) : []

  return (
    <div style={S.wrap}>
      <div style={S.brand}>
        <div style={S.brandName}>NEKTA</div>
        <div style={S.brandTag}>THE SCENE &gt; THE STREAMS</div>
      </div>

      <button style={S.spinBtn} onClick={() => setSpinning(s => !s)}>
        {spinning ? '⏸' : '▶'} <span style={{fontSize:'7px'}}>{spinning?'PAUSE':'SPIN'}</span>
      </button>

      <div style={{position:'relative',width:SIZE,height:SIZE,touchAction:'none'}}>
        {loading && <div style={S.loader}>LOADING...</div>}
        <canvas ref={canvasRef}
          style={{display:'block',cursor:'crosshair',opacity:visible?1:0,transition:'opacity 1.2s ease'}}
          onClick={onCanvasClick}
          onPointerDown={onPointerDown} onPointerUp={onPointerUp} onPointerMove={onPointerMove}
          onWheel={onWheel} onTouchStart={onTouchStart} onTouchMove={onTouchMove} onTouchEnd={onTouchEnd}
        />
      </div>

      {/* City sidebar — left side */}
      <div style={S.sidebar}>
        {HOTSPOTS.map(h => (
          <button key={h.name} onClick={() => flyTo(h)}
            style={{...S.si, opacity:activeHotspot?.name===h.name?1:0.45, background:activeHotspot?.name===h.name?'rgba(255,65,32,0.1)':'none'}}>
            <span style={{...S.dot,...(h.tier===2?S.dotBig:h.tier===1?S.dotMid:{})}}/>
            <span style={S.label}>{h.name}</span>
            {events[h.name]?.length > 0 && <span style={S.badge}>{events[h.name].length}</span>}
          </button>
        ))}
      </div>

      {/* Event panel */}
      {activeHotspot && (
        <div style={S.panel}>
          <div style={S.panelHeader}>
            <div>
              <div style={S.panelCity}>{activeHotspot.name}</div>
              <div style={S.panelDesc}>{activeHotspot.desc}</div>
            </div>
            <button onClick={dismiss} style={S.closeBtn}>✕</button>
          </div>
          {cityEvents.length > 0 ? (
            <div style={S.eventsWrap}>
              <div style={S.eventsTitle}>THIS WEEK</div>
              {cityEvents.map(ev => (
                <div key={ev.id} style={S.eventRow}>
                  <div style={S.eventName}>{ev.title}</div>
                  <div style={S.eventMeta}>
                    {ev.venue?.name}{ev.venue?.name && ' · '}
                    {new Date(ev.startTime).toLocaleDateString('en-GB',{weekday:'short',day:'numeric',month:'short'})}
                  </div>
                  {ev.artists && ev.artists.length > 0 && (
                    <div style={S.eventArtists}>{ev.artists.slice(0,3).map((a:any) => a.name).join(' · ')}</div>
                  )}
                </div>
              ))}
            </div>
          ) : (
            <div style={S.noEvents}>No live data · check RA for listings</div>
          )}
        </div>
      )}

      <div style={S.hint}>click a dot · drag · scroll to tilt · ctrl+scroll to zoom · space to pause</div>
    </div>
  )
}

const S: Record<string,React.CSSProperties> = {
  wrap:{width:'100%',minHeight:'100svh',background:'#0d0a09',display:'flex',alignItems:'center',justifyContent:'center',position:'relative',overflow:'hidden',fontFamily:'monospace'},
  brand:{position:'absolute',top:'24px',left:'24px',zIndex:10},
  brandName:{fontSize:'14px',fontWeight:700,letterSpacing:'0.22em',color:'#ff4120'},
  brandTag:{fontSize:'9px',color:'rgba(255,65,32,0.4)',letterSpacing:'0.12em',marginTop:'4px'},
  spinBtn:{position:'absolute',top:'24px',right:'20px',fontSize:'11px',color:'rgba(255,65,32,0.5)',background:'none',border:'1px solid rgba(255,65,32,0.2)',borderRadius:'4px',padding:'4px 10px',cursor:'pointer',zIndex:10,display:'flex',alignItems:'center',gap:'5px'},
  loader:{position:'absolute',inset:0,display:'flex',alignItems:'center',justifyContent:'center',color:'rgba(255,65,32,0.4)',fontSize:'10px',letterSpacing:'0.15em'},
  sidebar:{position:'absolute',left:'16px',top:'50%',transform:'translateY(-50%)',display:'flex',flexDirection:'column',gap:'3px',maxHeight:'80vh',overflowY:'auto',zIndex:10},
  si:{display:'flex',alignItems:'center',gap:'7px',cursor:'pointer',padding:'3px 8px',borderRadius:'4px',border:'none',transition:'all 0.2s'},
  dot:{width:'6px',height:'6px',borderRadius:'50%',background:'#ff4120',flexShrink:0,display:'inline-block'},
  dotMid:{width:'8px',height:'8px'},
  dotBig:{width:'11px',height:'11px',boxShadow:'0 0 8px #ff4120'},
  label:{fontSize:'9px',color:'#ff7050',letterSpacing:'0.08em',textTransform:'uppercase' as const,whiteSpace:'nowrap' as const},
  badge:{marginLeft:'2px',fontSize:'8px',background:'rgba(255,65,32,0.25)',color:'#ff7050',padding:'1px 4px',borderRadius:'3px'},
  panel:{position:'absolute',bottom:'28px',left:'50%',transform:'translateX(-50%)',background:'rgba(10,4,2,0.97)',border:'1px solid rgba(255,65,32,0.35)',borderRadius:'8px',padding:'14px 18px',minWidth:'280px',maxWidth:'360px',zIndex:20},
  panelHeader:{display:'flex',justifyContent:'space-between',alignItems:'flex-start'},
  panelCity:{fontSize:'12px',fontWeight:700,color:'#ff4120',letterSpacing:'0.18em',textTransform:'uppercase' as const},
  panelDesc:{fontSize:'9px',color:'rgba(255,130,100,0.55)',marginTop:'3px',letterSpacing:'0.05em'},
  closeBtn:{background:'none',border:'none',color:'rgba(255,65,32,0.5)',cursor:'pointer',fontSize:'14px',padding:'0 0 0 12px',lineHeight:1},
  eventsWrap:{marginTop:'12px',borderTop:'1px solid rgba(255,65,32,0.15)',paddingTop:'10px'},
  eventsTitle:{fontSize:'8px',color:'rgba(255,65,32,0.45)',letterSpacing:'0.18em',marginBottom:'8px'},
  eventRow:{marginBottom:'8px',paddingBottom:'8px',borderBottom:'1px solid rgba(255,65,32,0.08)'},
  eventName:{fontSize:'10px',color:'rgba(255,200,150,0.9)',letterSpacing:'0.04em',lineHeight:'1.4'},
  eventMeta:{fontSize:'8px',color:'rgba(255,100,50,0.5)',marginTop:'2px',letterSpacing:'0.06em'},
  eventArtists:{fontSize:'8px',color:'rgba(255,150,80,0.6)',marginTop:'2px',letterSpacing:'0.04em'},
  noEvents:{fontSize:'9px',color:'rgba(255,65,32,0.3)',marginTop:'10px',letterSpacing:'0.06em'},
  hint:{position:'absolute',bottom:'10px',left:'50%',transform:'translateX(-50%)',fontSize:'8px',color:'rgba(255,65,32,0.2)',letterSpacing:'0.1em',textTransform:'uppercase',whiteSpace:'nowrap'},
}
