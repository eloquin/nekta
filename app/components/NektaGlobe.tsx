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
  // Force-mark coastal cities that sit in harbour/bay pixels
  const mark = (lat: number, lon: number) => {
    const c = Math.floor((lon+180)/360*MAP_W)
    const r = Math.floor((90-lat)/180*MAP_H)
    for (let dr=-1;dr<=1;dr++) for (let dc=-1;dc<=1;dc++) {
      const rr=r+dr, cc=c+dc
      if (rr>=0&&rr<MAP_H&&cc>=0&&cc<MAP_W) result[rr*MAP_W+cc]=1
    }
  }
  mark(40.71,-74.01)  // New York
  mark(13.75,100.50)  // Bangkok
  mark(35.69,139.69)  // Tokyo
  mark(-37.81,144.96) // Melbourne
  mark(37.57,126.98)  // Seoul
  mark(-22.91,-43.17) // Rio
  mark(-23.55,-46.63) // Sao Paulo
  mark(38.91,1.43)    // Ibiza
  mark(9.73,100.01)   // Koh Phangan
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
