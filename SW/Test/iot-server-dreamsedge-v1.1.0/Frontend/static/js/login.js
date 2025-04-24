//--------------------------------------------------------------
// 
//--------------------------------------------------------------
$(document).ready(function() 
{
    $("#loginForm").submit(function (event) 
    {
        event.preventDefault(); // Prevent form from reloading the page

        $("#loginButton").prop("disabled", true).text("Logging in..."); // Disable button

        $.post("/api/login", $(this).serialize(), function (response) 
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