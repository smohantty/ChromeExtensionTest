
// content.js

chrome.runtime.onMessage.addListener(async (request, sender, sendResponse) => {
    if (request.action === 'getTabInfo') {
      try {
        const tabInfo = await getTabInfo();
        sendResponse(tabInfo);
      } catch (error) {
        console.error('Error processing getTabInfo request:', error);
        sendResponse({ error: 'Failed to get tab information' });
      }
    }
  });
  
  async function getTabInfo() {
    return new Promise((resolve, reject) => {
      // Replace this logic with your actual tab information retrieval logic
      const tabInfo = { url: 'example.com' };
      resolve(tabInfo);
      // If there's an error, reject the Promise
      // reject(new Error('Failed to get tab information'));
    });
  }
  
  