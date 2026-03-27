// app/api/events/route.ts
// Fetches live electronic music events from Resident Advisor GraphQL
// Note: RA may require browser-like headers — works from Next.js server

import { NextResponse } from 'next/server'

const RA_AREAS: Record<string, number> = {
  'London': 13, 'Berlin': 34, 'Ibiza': 37, 'Amsterdam': 9,
  'Paris': 11, 'Barcelona': 8, 'New York': 4, 'Chicago': 62,
  'Detroit': 63, 'LA': 5, 'Toronto': 29, 'Bogota': 368,
  'Rio': 97, 'Sao Paulo': 96, 'Bangkok': 183, 'Melbourne': 61,
  'Tokyo': 102, 'Bali': 384, 'Cape Town': 345, 'Prague': 55,
  'Zurich': 57, 'Dubai': 348, 'Seoul': 186, 'Koh Phangan': 384,
}

// Correct RA GraphQL query format (from community scrapers)
const QUERY = `
query GetEventListings($filters: FilterInputDtoInput, $pageSize: Int, $page: Int) {
  eventListings(filters: $filters, pageSize: $pageSize, page: $page) {
    data {
      id
      listingDate
      event {
        id
        title
        date
        startTime
        venue { name }
        artists { name }
      }
    }
    totalResults
  }
}`

export async function GET() {
  const today = new Date()
  const dateFrom = today.toISOString().split('T')[0]
  const dateTo = new Date(today.getTime() + 7*24*60*60*1000).toISOString().split('T')[0]

  const cities = ['London','Berlin','Ibiza','Amsterdam','Barcelona','New York','Tokyo','Melbourne','Bangkok','Paris','Bristol','LA','Chicago','Detroit']
  const results: Record<string, any[]> = {}

  await Promise.allSettled(cities.map(async city => {
    const areaId = RA_AREAS[city]
    if (!areaId) return
    try {
      const res = await fetch('https://ra.co/graphql', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Referer': 'https://ra.co/events',
          'Origin': 'https://ra.co',
          'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 Chrome/122.0.0.0 Safari/537.36',
        },
        body: JSON.stringify({
          query: QUERY,
          variables: {
            filters: {
              areas: { id: areaId },
              listingDate: { gte: dateFrom, lte: dateTo }
            },
            pageSize: 4,
            page: 1
          }
        }),
        next: { revalidate: 3600 }
      })
      if (!res.ok) return
      const json = await res.json()
      const listings = json?.data?.eventListings?.data ?? []
      if (listings.length > 0) {
        results[city] = listings.map((l: any) => ({
          id: l.event?.id ?? l.id,
          title: l.event?.title ?? 'Event',
          startTime: l.event?.startTime ?? l.listingDate,
          venue: l.event?.venue,
          artists: l.event?.artists ?? []
        }))
      }
    } catch { /* silently skip */ }
  }))

  // Return results (empty object if RA blocked — globe still works)
  return NextResponse.json(results, {
    headers: { 'Cache-Control': 's-maxage=3600, stale-while-revalidate=86400' }
  })
}
