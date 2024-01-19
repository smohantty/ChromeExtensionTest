
// content.js

chrome.runtime.onMessage.addListener(async (request, sender, sendResponse) => {
    if (request.action === 'getUrlInfo') {
      try {
        const urlInfo = await getUrlInfo();
        sendResponse(urlInfo);
      } catch (error) {
        console.error('Error processing getUrlInfo request:', error);
        sendResponse({ error: 'Failed to get tab information' });
      }
    }
  });
  
  async function getUrlInfo() {
    return new Promise((resolve, reject) => {
      // Replace this logic with your actual tab information retrieval logic
      const urlInfo = { url: 'example.com' };
      resolve(urlInfo);
      // If there's an error, reject the Promise
      // reject(new Error('Failed to get tab information'));
    });
  }
  
  