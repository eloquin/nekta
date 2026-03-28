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
