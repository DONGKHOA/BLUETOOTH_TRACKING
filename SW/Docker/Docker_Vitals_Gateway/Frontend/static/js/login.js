$(document).ready(function() 
{
    // Run session check every 5 seconds
    setInterval(checkSession, 5000);

    $("#loginForm").submit(function (event) 
    {
        event.preventDefault(); // Prevent form from reloading the page

        $("#loginButton").prop("disabled", true).text("Logging in..."); // Disable button

        $.post("/login", $(this).serialize(), function (response) 
        {
            console.log("Server response:", response); // Debugging log

            if (response.success) 
            {
                console.log("Redirecting to:", response.redirect);
                window.location.href = response.redirect; // Redirect on success
            } 
            else 
            {
                $("#authMessage").css("color", "red").text(response.message);
                $("#loginButton").prop("disabled", false).text("Login"); // Re-enable button
            }
        })
        .fail(function () 
        {   
            console.error("AJAX error:", xhr.responseText);
            $("#authMessage").css("color", "red").text("Server error! Please try again.");
            $("#loginButton").prop("disabled", false).text("Login");
        });
    });
});

function checkSession() {
    $.get("/check_session", function(response) {
        if (!response.valid) {
            alert(response.message);

            // Disable all buttons and prevent further actions
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
