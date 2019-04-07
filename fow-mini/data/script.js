'use strict';

var setupInfo = {
    ssid: null,
    password: null,
    networkRequiresPassword: true,
    resetInfo: function () {
        this.ssid = this.password = null;
        this.networkRequiresPassword = true;
    },
    attemptConnection: function () {
        let url = `/connect?ssid=${this.ssid}&password=${this.password || ""}`;

        return fetch(url).then(r => r.ok);
    },
};

window.onload = () => {
    let ssidSelector = document.querySelector("#networks");
    let passwordInput = document.querySelector("#pass");

    let ssidNextButton = ssidSelector.parentNode.querySelector("#next");
    let passwordNextButton = passwordInput.parentNode.querySelector("#next");

    let successStep = document.querySelector("#successStep");
    let failedStep = document.querySelector("#failedStep");
    let welcomeStep = document.querySelector("#welcomeStep");

    let dispNoNetworksFound = () => {
        ssidSelector.innerhtml = "<option>No networks found!</option>";
    }

    window.ssidSelectorOnChange = (s) => {
        if (!s) return;

        setupInfo.ssid = s.dataset.ssid;
        setupInfo.networkRequiresPassword = s.dataset.passwordRequired || true;

        setupInfo.password = "";
        passwordInput.value = "";
        passwordNextButton.disabled = true;

        if (setupInfo.ssid) {
            ssidNextButton.disabled = false;
        }
    };

    let unhideAndFocus = (step) => {
        if (step) {
            step.classList.remove("fadeAway");
            step.classList.remove("hidden");
        }
        let focusMe = step.querySelector(".focusMe");
        if (focusMe != null) setTimeout(focusMe.focus(), 0);
    };

    fetch("/networks").then(r => r.text().then(body => {
        if (!body) {
            dispNoNetworksFound();
            return;
        }

        let lines = body.split("\n");
        if (lines.length <= 1) {
            dispNoNetworksFound();
            return;
        }

        ssidSelector.innerHTML = "";
        ssidSelector.disabled = false;
        ssidSelector.onchange = (s) => {
            window.ssidSelectorOnChange(s.explicitOriginalTarget);
        };
        ssidSelector.onclick = () => {
            window.ssidSelectorOnChange(ssidSelector.options[ssidSelector.selectedIndex]);
        };

        let openNetworksList = lines[0].split(",");
        for (let i = 1; i < lines.length; i++) {
            if (lines[i] == "") continue;

            let passwordRequired = true;
            for (const openIdx of openNetworksList)
                if (new String(i - 1) == openIdx) passwordRequired = false;

            ssidSelector.innerHTML += `<option data-password-required="${passwordRequired}" data-ssid="${lines[i]}">${lines[i]} ${passwordRequired ? "🔒" : ""}</option>`;
        }
    }));

    let infoBox = document.querySelector("#infoBox");
    fetch("/info").then(r => {
        infoBox.innerText = "(c) Declan Freeman-Gleason and other contributors. This software is licensed under the GNU GPL v3 or above. See /LICENSE.\n\n"
        if (r.ok) r.text().then(text => infoBox.innerText += text);
        else infoBox.innerText += "Couldn't get info from /info. Are you connected?";
    });

    document.querySelector("#infoButton").onclick = () => {
        infoBox.classList.toggle("slideUpFade");

        let logos = document.querySelectorAll(".FT");
        for (const e of logos)
            e.classList.toggle("centerLarge");
    };

    passwordInput.oninput = (e) => {
        passwordNextButton.disabled = passwordInput.value == "";
        setupInfo.password = passwordInput.value;
    };

    successStep.querySelector("#redo").onclick = () => {
        successStep.classList.add("hidden");
        unhideAndFocus(welcomeStep);
    };

    successStep.querySelector("#exitSetup").onclick = () => {
        successStep.classList.add("fadeAway");
        fetch("/exitsetup").then((r) => {
            document.querySelector("#exitStep > .loadingSpinner").classList.add("hidden");

            let exitStatus = document.querySelector("#exitStatus");
            exitStatus.classList.remove("hidden");
            if (r.ok) exitStatus.textContent = "Successfully exited setup mode! You can safely close this page.";
            else {
                exitStatus.textContent = "Couldn't exit setup mode. You can go back and retry if you want.";
                document.querySelector("#exitStep > #back").classList.remove("hidden");
            }
        });
    };

    failedStep.querySelector("#redo").onclick = () => {
        failedStep.classList.add("hidden");
        unhideAndFocus(document.querySelector("#reviewStep"));
    };

    let steps = document.querySelectorAll(".step");
    for (let i = 0; i < steps.length; i++) {
        let backButton = steps[i].querySelector("#back");
        if (backButton) backButton.onclick = () => {
            steps[i].classList.add("hidden");

            let idx = i - 1;
            if (steps[i].id === "reviewStep" && setupInfo.networkRequiresPassword == "false") idx--;

            unhideAndFocus(steps[idx]);
        };

        let nextButton = steps[i].querySelector("#next");
        if (nextButton) nextButton.onclick = () => {
            if (nextButton.disabled) return false;

            steps[i].classList.add("fadeAway");

            let nextIdx = i + 1;

            if (steps[nextIdx].id === "passwordStep" && setupInfo.networkRequiresPassword == "false")
                nextIdx++;

            let ssidConfirm = steps[nextIdx].querySelector("#ssidConfirm");
            let passwordConfirm = steps[nextIdx].querySelector("#passwordConfirm");
            if (ssidConfirm && passwordConfirm) {
                ssidConfirm.value = setupInfo.ssid;
                passwordConfirm.value = setupInfo.password;
            }

            if (steps[nextIdx].id === "connectStep") {
                setupInfo.attemptConnection().then(ok => {
                    steps[nextIdx].classList.add("hidden");
                    unhideAndFocus(ok ? successStep : failedStep);
                });
            }

            // Now that current element is hidden we can show the next one
            unhideAndFocus(steps[nextIdx]);
        };

        steps[i].addEventListener("animationend", (e) => {
            if (e.animationName !== "OpacityFadeOut") return;

            // We only want to set display none on the current element once it has faded fully
            steps[i].classList.add("hidden");        
        });
    }
};