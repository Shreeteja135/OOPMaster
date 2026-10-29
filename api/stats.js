const kbData = require('../data/kb_data.json');

module.exports = function handler(req, res) {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET, OPTIONS');
  res.setHeader('Content-Type', 'application/json; charset=utf-8');

  if (req.method === 'OPTIONS') {
    res.status(200).end();
    return;
  }

  const enCount = (kbData.en || []).length;
  const hiCount = (kbData.hi || []).length;
  const mrCount = (kbData.mr || []).length;

  res.status(200).json({
    status: 'running',
    server: 'OOP Master Vercel Cloud Serverless Engine',
    english_records: enCount,
    hindi_records: hiCount,
    marathi_records: mrCount,
    total_records: enCount + hiCount + mrCount
  });
};
