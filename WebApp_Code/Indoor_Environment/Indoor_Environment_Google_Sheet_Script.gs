// Function to get Indoor Enviroment data in JSON format and parse it to Google Sheet
// This code is triggered by time-driven trigger at 10 minutes apart. 
function getTemperatureJSONData() {
  
  // URL of the ThingSpeak API
  // Read all four fields together
  const url = "https://api.thingspeak.com/channels/1098898/feeds.json?results=8000&timescale=30&timezone=Asia%2FKuala_Lumpur";

  // Fetch the URL data
  const response = UrlFetchApp.fetch(url).getContentText();
  
  // Parse data as JSON
  const responseJSON = JSON.parse(response);
  const data = responseJSON.feeds; 
 
  // Arrange JSON data as spreadsheet data
  const ssData = data.map(r => [r.created_at,r.field1,r.field2,r.field3,r.field4]);
  
  // Since parsed JSON data keeps increasing as the system runs,
  // the parsed JSON data is processed to only obtain 48 data.
  // Firstly, reverse the data, so that the newest data is now on top.
  // Secondly, slice the data into having only 48 newest data.
  // Thirdly, reverse back the data, so now only 48 newest data is used. 
  const reverseData = ssData.reverse();  
  const sliceData = reverseData.slice(0,47);
  const newData = sliceData.reverse();
  
  // Write ssData to spreadsheet
  SpreadsheetApp.getActiveSpreadsheet().getSheetByName("Data").getRange(2, 1, newData.length, 5).setValues(newData);
}

