/** @type {import('next').NextConfig} */
const nextConfig = {
  reactStrictMode: true,
  eslint: {
    ignoreDuringBuilds: true
  },
  typescript: { ignoreBuildErrors: true, },
  images: {
    unoptimized: true,
  },
  output: 'export'
}

module.exports = nextConfig
