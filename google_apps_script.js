/**
 * ======================================================================================
 * GOOGLE APPS SCRIPT FOR ESP32 MACHINE RUNTIME TRACKER
 * ======================================================================================
 * 
 * Columns in Google Sheet:
 * [Column A: Date]  [Column B: ON Time]  [Column C: OFF Time]  [Column D: Running Time]
 * 
 * HOW THIS WORKS (POWER-CUT METHOD):
 * 1. When the machine turns ON, ESP32 powers up and sends status="ON".
 *    -> This appends a NEW row with Date, ON Time, initial OFF Time, and "0s" Running Time.
 * 
 * 2. While the machine runs, ESP32 sends status="RUNNING" every 10 seconds.
 *    -> This continuously updates the LAST row's OFF Time (Column C) and Running Time (Column D).
 * 
 * 3. When the machine turns OFF, power is cut!
 *    -> The last recorded OFF Time and Running Time remain permanently saved on that row.
 * 
 * 4. When the machine turns ON next time:
 *    -> A brand new row is created automatically for the new session.
 * 
 * ======================================================================================
 * DEPLOYMENT INSTRUCTIONS:
 * 1. In your Google Sheet, click "Extensions" -> "Apps Script".
 * 2. Delete any existing code, and paste this entire script.
 * 3. Click "Save" (disk icon).
 * 4. Click "Deploy" (blue button at the top right) -> "Manage deployments".
 * 5. Click the pencil icon (Edit) on your current deployment.
 * 6. Under "Version", select "New version".
 * 7. Click "Deploy" and "Done".
 * ======================================================================================
 */

function doGet(e) {
  try {
    // Open the sheet (Sheet1 or active sheet)
    var spreadsheet = SpreadsheetApp.getActiveSpreadsheet();
    var sheet = spreadsheet.getSheetByName("Sheet1");
    if (!sheet) {
      sheet = spreadsheet.getActiveSheet();
    }

    // Read incoming parameters from ESP32
    var status  = (e && e.parameter && e.parameter.status) ? e.parameter.status : "";
    var runtime = (e && e.parameter && e.parameter.runtime) ? e.parameter.runtime : "0s";

    // Clean up runtime formatting (e.g., "1m_15s" -> "1m 15s")
    runtime = runtime.replace(/_/g, " ");

    // Current Date and Time in the Sheet's timezone
    var timeZone = spreadsheet.getSpreadsheetTimeZone() || Session.getScriptTimeZone();
    var now = new Date();
    var formattedDate = Utilities.formatDate(now, timeZone, "dd/MM/yyyy");
    var formattedTime = Utilities.formatDate(now, timeZone, "HH:mm:ss");

    if (status === "ON") {
      // Machine turned ON: Create a new row
      // Columns: [A: Date, B: ON Time, C: OFF Time, D: Running Time]
      sheet.appendRow([formattedDate, formattedTime, formattedTime, runtime]);
      return ContentService.createTextOutput("OK: ON logged");
    } 
    else if (status === "RUNNING") {
      // Machine is running: update the latest row with the current timestamp & elapsed runtime
      var lastRow = sheet.getLastRow();
      
      if (lastRow > 1) {
        sheet.getRange(lastRow, 3).setValue(formattedTime); // Column C: OFF Time
        sheet.getRange(lastRow, 4).setValue(runtime);       // Column D: Running Time
        return ContentService.createTextOutput("OK: RUNNING updated");
      } else {
        // If sheet has only headers, append the first data row
        sheet.appendRow([formattedDate, formattedTime, formattedTime, runtime]);
        return ContentService.createTextOutput("OK: First row created");
      }
    }

    return ContentService.createTextOutput("OK: No status action");
  } catch (error) {
    return ContentService.createTextOutput("Error: " + error.toString());
  }
}
