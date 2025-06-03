$(document).ready(function () {
    fetchLogs(); // Load logs initially
    setInterval(fetchLogs, 1000); // Auto-refresh every 5 seconds

    // Refresh logs manually
    $("#refreshLogs").click(function () {
        fetchLogs();
    });

    // Load current logging config when page loads
    $.get("/get_logging_config", function(response) {
        if (response.success) {
            $("#currentMaxBytes").text(response.maxBytes / (1024 * 1024));  // Convert bytes to MB
            $("#currentBackupCount").text(response.backupCount);
            $("#maxBytes").val(response.maxBytes / (1024 * 1024));  // Pre-fill input fields
            $("#backupCount").val(response.backupCount);
        }
    });

    // Handle updating logging configuration
    $("#configForm").submit(function(event) {
        event.preventDefault();
        $.post("/update_logging_config", $(this).serialize(), function(response) {
            $("#configMessage").text(response.message).css("color", response.success ? "green" : "red");
            if (response.success) {
                $("#currentMaxBytes").text($("#maxBytes").val());
                $("#currentBackupCount").text($("#backupCount").val());
            }
        }, "json");
    });

    // Clear logs
    $("#clearLogs").click(function () {
        $.post("/clear_logs", function (response) {
            if (response.success) {
                $("#logMessages").html("<p>Logs cleared successfully.</p>");
            } else {
                $("#logMessages").html("<p>Error clearing logs.</p>");
            }
        }).fail(function () {
            $("#logMessages").html("<p>Error connecting to the server.</p>");
        });
    });
});

// Function to fetch logs from the backend
function fetchLogs() {
    $.get("/get_logs", function (response) {
        let logContainer = $("#logMessages");
        logContainer.empty();  // Clear old logs from UI first

        if (response.success) {
            if (response.logs.length === 0) {
                logContainer.append("<p>No logs available.</p>");  // Show message when logs are expired
            } else {
                response.logs.forEach(log => {
                    logContainer.append(`<p>${log}</p>`);
                });
            }
        } else {
            logContainer.append("<p>Error loading logs.</p>");
        }

        if (isScrolledToBottom) {
            logContainer.scrollTop(logContainer[0].scrollHeight);
        }
        
    }).fail(function () {
        $("#logMessages").empty().append("<p>Failed to connect to the server.</p>");
    });
}

// Function to update logs without duplicates
function updateLogs(logs) {
    let logContainer = $("#logMessages");
    let existingLogs = logContainer.children("p").map(function () {
        return $(this).text();
    }).get();

    let newLogs = logs.filter(log => !existingLogs.includes(log)); // Filter out duplicates

    if (newLogs.length > 0) {
        let shouldScroll = logContainer[0].scrollHeight - logContainer.scrollTop() === logContainer.outerHeight();

        newLogs.forEach(log => {
            logContainer.append(`<p>${log}</p>`);
        });

        // Auto-scroll only if the user is already at the bottom
        if (shouldScroll) {
            logContainer.scrollTop(logContainer[0].scrollHeight);
        }
    }
}