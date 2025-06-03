$(document).ready(function () {

    checkServerStatus();

    loadUploadedFilesAndEndpoint();

    fetchFacilityList()
    
    // Call update function for each file input
    updateFileInfo("file1", "fileInfo1");
    updateFileInfo("file2", "fileInfo2");
    updateFileInfo("file3", "fileInfo3");

    loadExistingFiles();

    setInterval(checkServerSession, 5000);

    // Start Server Button Click
    $("#startServer").click(function () {
        $.post("/start_server", function (response) {
            $("#serverMessage").html("<p style='color: green;'>" + response.message + "</p>");
            checkServerStatus();
        }).fail(function () {
            $("#serverMessage").html("<p style='color: red;'>Error starting the server.</p>");
        });
    });

    // Stop Server Button Click
    $("#stopServer").click(function () {
        $.get("/stop_server", function (response) {
            $("#serverMessage").html("<p style='color: green;'>" + response.message + "</p>");
            checkServerStatus();
        }).fail(function () {
            $("#serverMessage").html("<p style='color: red;'>Error stopping the server.</p>");
        });
    });

    // Handle Authentication Form Submit
    $("#AuthenticateForm").submit(function (event) {
        event.preventDefault(); // Prevent page reload

        $("#AuthenticateButton").prop("disabled", true).text("Authenticating..."); // Disable button

        $.post("/authenticate", $(this).serialize(), function (response) {
            if (response.success) {
                // Login successful → Redirect
                $("#authMessage").css("color", "green").text("Login successful! Redirecting...");
                setTimeout(() => {
                    window.location.href = response.redirect;
                }, 1000); // Redirect after 1 second
            } else {
                // Wrong login → Show error message instead of redirecting
                $("#authMessage").css("color", "red").text("Wrong username or password.");
                $("#AuthenticateButton").prop("disabled", false).text("Authenticate"); // Re-enable button
            }
        }).fail(function (xhr) {
            // Server error → Show error instead of redirecting
            $("#authMessage").css("color", "red").text("Server error! Please try again.");
            $("#AuthenticateButton").prop("disabled", false).text("Authenticate");
        });
    });

    // Handle Facility ID Submission
    $("#facilityForm").submit(function (event) {
        event.preventDefault(); // Prevent page reload

        var selectedId = $("#facility_id").val(); // Get selected facility ID

        if (!selectedId) {
            $("#facilityMessage").html("<p style='color: red;'>Please select a Facility ID!</p>");
            return;
        }

        $.post("/select_facility_id", { facility_id: selectedId }, function (response) {
            if (response.success) {
                $("#facilityMessage").html("<p style='color: green;'>Facility ID = \"" + selectedId + "\" updated successfully!</p>");
            } else {
                $("#facilityMessage").html("<p style='color: red;'>" + response.message + "</p>");
            }
        }).fail(function () {
            $("#facilityMessage").html("<p style='color: red;'>Error updating Facility ID.</p>");
        });
    });

    // Update displayed file name when user selects a file
    $("#file1").change(function () {
        $("#fileInfo1").text(this.files.length ? this.files[0].name : "No file chosen");
    });
    $("#file2").change(function () {
        $("#fileInfo2").text(this.files.length ? this.files[0].name : "No file chosen");
    });
    $("#file3").change(function () {
        $("#fileInfo3").text(this.files.length ? this.files[0].name : "No file chosen");
    });
    
    // Handle Certificate Upload
    $("#uploadForm").submit(function (event) {
        event.preventDefault(); // Prevent page refresh
        var formData = new FormData(this);

        $.ajax({
            url: "/upload_cert",
            type: "POST",
            data: formData,
            processData: false,
            contentType: false,
            success: function (response) {
                if (response.success) {
                    // Update file paths dynamically based on uploaded files
                    if (response.updated_paths.file1) {
                        $("#fileInfo1").text(response.updated_paths.file1);
                    }
                    if (response.updated_paths.file2) {
                        $("#fileInfo2").text(response.updated_paths.file2);
                    }
                    if (response.updated_paths.file3) {
                        $("#fileInfo3").text(response.updated_paths.file3);
                    }

                    // Display success message
                    $("#uploadMessage").html("<p style='color: green;'>Uploaded: " + response.uploaded_files.map(f => `"${f}"`).join(", ") + " successfully!</p>");

                    // Refresh the uploaded files list dynamically
                    loadUploadedFilesAndEndpoint();

                } else {
                    // Display error message for invalid file type
                    $("#uploadMessage").html("<p style='color: red;'>" + response.message + "</p>");
                }

                // Clear file input fields
                $("#file1").val(""); 
                $("#file2").val(""); 
                $("#file3").val(""); 
            },
            error: function (xhr) {
                // Display error message for invalid file type
                var errorMessage = xhr.responseJSON ? xhr.responseJSON.message : "Error uploading file.";
                $("#uploadMessage").html("<p style='color: red;'>" + errorMessage + "</p>");
            }
        });
    });

    // Handle AWS IoT Endpoint Update
    $("#awsEndpointForm").submit(function (event) {
        event.preventDefault();
        let newEndpoint = $("#aws_iot_endpoint").val().trim();

        if (!newEndpoint) {
            $("#AWSEndPoint").html("<p style='color: red;'>AWS IoT Endpoint cannot be empty!</p>");
            return;
        }

        $.post("/update_aws_endpoint", { aws_iot_endpoint: newEndpoint })
            .done(function (response) {
                $("#AWSEndPoint").html(`<p style='color: green;'>AWS IoT Endpoint updated successfully!</p>`);
                $("#awsEndpointDisplay").text(response.new_endpoint);
                $("#aws_iot_endpoint").val("");
                
                loadUploadedFilesAndEndpoint()
            })
            .fail(function () {
                $("#AWSEndPoint").html("<p style='color: red;'>Error updating AWS IoT Endpoint.</p>");
            });
    });

    // Handle AWS Connection Check
    $("#AWSCheckingButton").click(function () {
        console.log("Checking AWS connection...");

        $("#AWSEndPoint").html("<p style='color: blue;'>Checking connection...</p>");
        $("#AWSCheckingButton").prop("disabled", true);

        $.get("/check_aws_connection")
            .done(function (response) {
                $("#AWSCheckingButton").prop("disabled", false);
                let statusText = {
                    success: "<p style='color: green;'>Connection successful!</p>",
                    failure: "<p style='color: red;'>Connection failed!</p>",
                    timeout: "<p style='color: orange;'>Connection check timed out!</p>"
                };
                $("#AWSEndPoint").html(statusText[response.status] || "<p style='color: red;'>Unknown status.</p>");
            })
            .fail(function () {
                $("#AWSEndPoint").html("<p style='color: red;'>Request error calling /check_aws_connection</p>");
            });
    });

    // Handle AWS IoT Connection Check Button Click
    $("#AWSCheckingButton").click(function () {
        console.log("Check Connection button clicked!"); // Debugging

        // 1) Clear old status text
        $("#AWSEndPoint").empty();
        
        // 2) Show a "checking..." message or spinner
        $("#loadingSpinner").show();

        // Disable the button so it can’t be clicked again during the check
        $(this).prop("disabled", true);

        // Send request to check AWS connection
        $.get("/check_aws_connection", function (response) {
            // Hide spinner once we have a response
            $("#loadingSpinner").hide();
            $("#AWSCheckingButton").prop("disabled", false); // Re-enable button

            // Handle response status
            let message = "";
            if (response.status === "success") {
                message = "<p style='color: green;'>Connection successful!</p>";
            } else if (response.status === "failure") {
                message = "<p style='color: red;'>Connection failed!</p>";
            } else if (response.status === "timeout") {
                message = "<p style='color: orange;'>Connection check timed out!</p>";
            } else {
                message = "<p style='color: red;'>Unknown status returned.</p>";
            }

            // Update the status message
            $("#AWSEndPoint").html(message);
        })
        .fail(function () {
            $("#loadingSpinner").hide();
            $("#AWSEndPoint").html("<p style='color: red;'>Request error calling /check_aws_connection</p>");
        });
    });
})

// Fetch Facility List from API and Update UI
function fetchFacilityList() {
    console.log("Fetching facility list..."); // Debugging

    $.get("/facility_list", function(response) {
        console.log("Facility List Response:", response); // Debugging

        if (response.success) {
            var tableBody = $("#facilityTable tbody");
            tableBody.empty(); // Clear existing rows

            response.data.forEach(function(item) {
                var row = `
                    <tr>
                        <td>${item.id}</td>
                        <td>${item.name}</td>
                        <td>${item.address}</td>
                        <td>${item.timezone}</td>
                        <td>${item.tcp_server_name}</td>
                    </tr>
                `;
                tableBody.append(row);
            });

            $("#facilityCount").text(response.count);
        } else {
            $("#facilityMessage").html("<p style='color: red;'>No facilities found.</p>");
        }
    }).fail(function() {
        $("#facilityMessage").html("<p style='color: red;'>Error fetching facility list.</p>");
    });
}

// Update displayed file name when a new file is selected
function updateFileInfo(inputId, displayId) {
    $("#" + inputId).change(function () {
        if (this.files.length > 0) {
            let fileName = this.files[0].name;
            $("#" + displayId).text(fileName); // Show just file name before upload
        }
    });
}

// Load existing file paths from the server
function loadExistingFiles() {
    $.get("/get_existing_files", function (response) {
        if (response.success) {
            $("#fileInfo1").text(response.root_ca_path);
            $("#fileInfo2").text(response.private_key_path);
            $("#fileInfo3").text(response.cert_path);
        } else {
            console.error("Failed to load existing files.");
        }
    }).fail(function () {
        console.error("Error fetching existing files.");
    });
}

// Function to refresh the uploaded files and AWS IoT Endpoint display dynamically
function loadUploadedFilesAndEndpoint() {
    $.get("/get_uploaded_files", function (response) {
        var fileListElement = $("#uploadedFilesList");
        fileListElement.empty();

        if (response.files.length > 0) {
            response.files.forEach(function (file) {
                fileListElement.append("<li>" + file + "</li>");
            });
        } else {
            fileListElement.append("<li>No files uploaded</li>");
        }

        // Update AWS IoT Endpoint in "Uploaded Files & AWS IoT Endpoint" section
        $("#awsEndpointDisplay").text(response.aws_endpoint);
    }).fail(function () {
        console.error("Error fetching uploaded files and endpoint.");
    });
}

function checkServerStatus() {
    $.get("/button_server_status", function (response) {
        console.log("Server status response:", response); // Debugging

        if (response.running) {
            // Server is running → Disable everything except Stop Server
            $("#startServer").prop("disabled", true).css("opacity", "0.5");
            $("#stopServer").prop("disabled", false).css("opacity", "1");

            // Disable all other buttons
            $("#AuthenticateButton").prop("disabled", true).css("opacity", "0.5"); 
            $("#AWSUpdateButton").prop("disabled", true).css("opacity", "0.5");
            $("#AWSCheckingButton").prop("disabled", true).css("opacity", "0.5");
            $("#UploadButton").prop("disabled", true).css("opacity", "0.5");
            $("#FacilityIDButton").prop("disabled", true).css("opacity", "0.5"); // Disable Select Facility Submit Button

        } else {
            // Server is stopped → Enable everything
            $("#stopServer").prop("disabled", true).css("opacity", "0.5");
            $("#AuthenticateButton").prop("disabled", false).css("opacity", "1"); 
            $("#AWSUpdateButton").prop("disabled", false).css("opacity", "1");
            $("#AWSCheckingButton").prop("disabled", false).css("opacity", "1");
            $("#UploadButton").prop("disabled", false).css("opacity", "1");
            $("#FacilityIDButton").prop("disabled", false).css("opacity", "1"); // Enable Select Facility Submit Button

            // Enable Start Server only if allowed
            if (response.enable_start_button) {
                $("#startServer").prop("disabled", false).css("opacity", "1"); // Enable Start Server if allowed
            } else {
                $("#startServer").prop("disabled", true).css("opacity", "0.5"); // Keep Start Server disabled
            }
        }
    }).fail(function () {
        console.error("Failed to fetch server status");
    });
}

function checkServerSession() {
    $.get("/check_session", function(response) {
        if (!response.valid) {
            alert(response.message);

            // Disable buttons and prevent further actions
            $("button").prop("disabled", true).css("opacity", "0.5");

            // Redirect user to login page after 1 second
            setTimeout(function() {
                window.location.href = "/login";
            }, 1000);
        }
    }).fail(function () {
        console.error("Error checking session.");
    });
}