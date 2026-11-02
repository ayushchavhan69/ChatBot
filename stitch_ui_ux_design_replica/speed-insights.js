/**
 * Vercel Speed Insights Integration
 * Loads and initializes Speed Insights for performance tracking
 */

(function() {
  // Speed Insights will be loaded from the Vercel CDN
  // This script injects the tracking code as per Vercel documentation
  
  if (typeof window === 'undefined') return;

  // Initialize queue for events before library loads
  window.siq = window.siq || [];
  
  // Create and inject the script
  const script = document.createElement('script');
  script.defer = true;
  
  // The script path follows Vercel's standard deployment pattern
  // When deployed on Vercel, this will automatically load from /_vercel/speed-insights/script.js
  // For local development, this won't track (as per Speed Insights design)
  script.src = '/_vercel/speed-insights/script.js';
  
  // Add error handling
  script.onerror = function() {
    console.debug('Speed Insights: Not loaded (likely in development or not deployed on Vercel)');
  };
  
  // Inject the script
  if (document.head) {
    document.head.appendChild(script);
  } else {
    // If head is not ready, wait for DOMContentLoaded
    document.addEventListener('DOMContentLoaded', function() {
      document.head.appendChild(script);
    });
  }
})();
