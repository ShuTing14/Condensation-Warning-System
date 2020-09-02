// Function to get WeatherBit API data in JSON format and parse it to Google Sheet.
// This code is triggered by time-driven trigger at 10 minutes apart.
function getWeatherBitJSONData() {
  
  // URL of the Weatherbit API
  const url = "https://api.weatherbit.io/v2.0/forecast/hourly?city=Klang,MY&key=e79f7353942a4e1592806b109d929689&hours=24";

  // Fetch the URL data
  const response = UrlFetchApp.fetch(url).getContentText();
  
  // Parse data as JSON
  const responseJSON = JSON.parse(response);
  const data = responseJSON.data; 
 
  // Arrange JSON data as spreadsheet data
  const ssData = data.map(r => [r.timestamp_local,r.temp,r.rh,r.dewpt]);
  
  // Write ssData to spreadsheet  
  SpreadsheetApp.getActiveSpreadsheet().getSheetByName("ForecastData").getRange(2, 1, ssData.length, 4).setValues(ssData);
}
