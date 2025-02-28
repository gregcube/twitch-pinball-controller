const WebSocket = require('ws');
const axios = require('axios');

const TWITCH_CLIENT_ID = '';
const TWITCH_TOKEN = '';
const TWITCH_CHANNEL_ID = '';
const ESP32_IP = '';

const ws = new WebSocket('wss://eventsub.wss.twitch.tv/ws');
let sessionId;

ws.on('open', () => console.log('Connected to Twitch EventSub'));

ws.on('message', async (data) => {
  const msg = JSON.parse(data);

  if (msg.metadata.message_type === 'session_welcome') {
    sessionId = msg.payload.session.id;
    console.log('Session ID:', sessionId);
    await subscribeToChat();
  }

  if (msg.metadata.message_type === 'notification' && msg.payload.subscription.type === 'channel.chat.message') {
    const chatMessage = msg.payload.event.message.text.toLowerCase();

    if (chatMessage === 'left flip') triggerGPIO(12);
    else if (chatMessage === 'right flip') triggerGPIO(14);
    else if (chatMessage === 'start') triggerGPIO(27);
  }
});

ws.on('error', (err) => console.error('WebSocket error:', err));
ws.on('close', () => console.log('WebSocket closed'));

async function subscribeToChat() {
  const url = 'https://api.twitch.tv/helix/eventsub/subscriptions';

  const headers = {
    'Client-ID': TWITCH_CLIENT_ID,
    'Authorization': `Bearer ${TWITCH_TOKEN}`,
    'Content-Type': 'application/json'
  };

  const body = {
    type: 'channel.chat.message',
    version: '1',
    condition: {
      broadcaster_user_id: TWITCH_CHANNEL_ID,
      user_id: TWITCH_CHANNEL_ID
    },
    transport: {
      method: 'websocket',
      session_id: sessionId
    }
  };

  try {
    const res = await axios.post(url, body, { headers });
    console.log('Subscribed to chat:', res.data);
  }
  catch (err) {
    console.error('Subscription error:', JSON.stringify(err.response?.data, null, 2) || err.message);
  }
}

async function triggerGPIO(gpio) {
  const url = `http://${ESP32_IP}/trigger`;
  const data = { gpio };

  try {
    await axios.post(url, data, { headers: { 'Content-Type': 'application/json' } });
    console.log(`Triggered GPIO ${gpio}`);
  }
  catch (err) {
    console.error(`Failed to trigger GPIO ${gpio}:`, err.message);
  }
}

