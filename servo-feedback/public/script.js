var msgEl,
	servoEl,
	socket = null;
var lastPos = 0;

const MsgError = 1;
const MsgMove = 2;
const MsgMoveResult = 3;
const MsgPosition = 4;
const MsgTwist1 = 5;
const MsgTwist2 = 6;
const MsgTwist3 = 7;
const MsgTwist4 = 8;
const MsgStart = 9;
const MsgStopp = 10;

const stepSize = 10; // How much +/- buttons move by

if (document.readyState != 'loading') onDocumentReady();
else document.addEventListener('DOMContentLoaded', onDocumentReady);

function handleCommand(d) {
	switch (d.integer) {
		case MsgPosition:
			onPositionUpdate(d);
			break;
	}
}
//------My functions-----------

function twist1() {
	send(MsgTwist1, 0);
}
function twist2() {
	send(MsgTwist2, 0);
}
function twist3() {
	send(MsgTwist3, 0);
}
function twist4() {
	send(MsgTwist4, 0);
}

// ties data-hoverfloat to sound-function
function hoverElements() {
	document.querySelectorAll('.all').forEach(element => {
		element.addEventListener('mouseover', event => {
			console.log(event.target.dataset.hoverfloat);
			send(MsgStart, event.target.dataset.hoverfloat);
		});
		element.addEventListener('mouseleave', event => {
			send(MsgStopp, 0);
		});
	});
}

function initRotate() {
	const elements = document.querySelectorAll('.darkblue');
	elements.forEach(element => {
		let intervalId;
		let degrees = 0;
		element.addEventListener('mouseover', event => {
			move(element)
				.rotate((degrees += 180))
				.end();
			intervalId = setInterval(() => {
				move(element)
					.rotate((degrees += 180))
					.end();
			}, 500);
		});
		element.addEventListener('mouseleave', event => {
			clearInterval(intervalId);
			degrees = 0;
			move(element)
				.rotate(0)
				.end();
		});
	});
}

function initColor() {
	const elements = document.querySelectorAll('.white');
	elements.forEach(element => {
		element.addEventListener('click', event => {
			element.style.borderTopColor = '#bcceff';
			element.style.borderBottomColor = '#bcceff';
			setTimeout(() => {
				element.style.borderTopColor = 'white';
				element.style.borderBottomColor = 'white';
			}, 2000);
		});
	});
}

function initMove() {
	const elements = document.querySelectorAll('.lightblue');
	elements.forEach(element => {
		element.addEventListener('mouseover', event => {
			element.style.transform = 'translateX(200px)';
			setTimeout(() => {
				element.style.transform = '';
			}, 2000);
		});
	});
}

function initLetters() {
	const elements = document.querySelectorAll('.letters');
	elements.forEach(element => {
		element.addEventListener('click', event => {
			document.querySelector('#n').style.transform = 'translate(-78px, 79px)';
			document.querySelector('#t').style.transform = 'translate(-34px, -169px)';
			document.querySelector('#e').style.transform = 'translate(-126px, -110px)';
			document.querySelector('#r').style.transform = 'translate(-114px, -110px)';
			document.querySelector('#a').style.transform = 'translate(-157px, 110px)';
			document.querySelector('#c').style.transform = 'translate(-132px, -32px)';
			document.querySelector('#t1').style.transform = 'translate(-265px, -103px)';
			setTimeout(() => {
				elements.forEach(element => (element.style.transform = ''));
			}, 2000);
		});
	});
}

function initTri4() {
	const element = document.querySelector('#tri4');
	element.addEventListener('click', event => {
		document.querySelectorAll('.white').forEach(element => {
			element.style.borderTopColor = '#bcceff';
			element.style.borderBottomColor = '#bcceff';
			setTimeout(() => {
				element.style.borderTopColor = 'white';
				element.style.borderBottomColor = 'white';
			}, 2000);
		});
	});
}

function initTri3() {
	const element = document.querySelector('#tri3');
	element.addEventListener('click', event => {
		document.querySelector('#tri1').style.transitionDelay = '500ms';
		document.querySelector('#tri2').style.transitionDelay = '1250ms';
		document.querySelector('#tri3').style.transitionDelay = '750ms';
		document.querySelector('#tri4').style.transitionDelay = '750ms';
		document.querySelector('#tri5').style.transitionDelay = '1000ms';
		document.querySelector('#tri6').style.transitionDelay = '1500ms';
		document.querySelector('#tri7').style.transitionDelay = '1000ms';
		document.querySelector('#tri12').style.transitionDelay = '500ms';
		document.querySelector('#tri13').style.transitionDelay = '1250ms';
		document.querySelector('#tri14').style.transitionDelay = '1500ms';
		document.querySelectorAll('.white').forEach(element => {
			element.style.borderTopColor = '#bcceff';
			element.style.borderBottomColor = '#bcceff';
			setTimeout(() => {
				element.style.borderTopColor = 'white';
				element.style.borderBottomColor = 'white';
			}, 3000);
			setTimeout(() => {
				element.style.transitionDelay = '';
			}, 6500);
		});
	});
}

// Received the position of the servo
function onPositionUpdate(d) {
	// We expect numbers between 0-180, but sometimes this can be a bit
	// screwy due to calibration errors. Therefore, first sanitise
	var pos = d.float;
	setStatus(`Position: ${pos}`);

	if (pos < 0) pos = 0;
	if (pos > 180) pos = 180;
	lastPos = pos; // Keep track of position

	// Expand 0..180 to 0..360
	pos = pos * 2;
	servoEl.style.transform = 'rotate(' + pos + 'deg)';
}

function setStatus(msg) {
	msgEl.innerText = msg;
}

function onDocumentReady() {
	initRotate();
	initColor();
	initMove();
	initLetters();
	initTri4();
	initTri3();
	socket = new ReconnectingWebsocket('ws://' + location.host + '/serial');
	servoEl = document.getElementById('servo');
	msgEl = document.getElementById('msg');

	socket.onmessage = function(evt) {
		// Debug: see raw received message
		//console.log(evt.data);

		// Parse message, assuming <Text,Int,Float>
		var d = evt.data.trim();
		if (d.charAt(0) == '<' && d.charAt(d.length - 1) == '>') {
			// Looks legit
			d = d.split(',');
			if (d.length == 3) {
				// Yes, it has three components as we hoped
				handleCommand({
					text: d[0].substr(1),
					integer: parseInt(d[1]),
					float: parseFloat(d[2].substr(0, d.length - 1)),
				});
				return;
			}
		}

		// Doesn't seem to be formatted correctly, just display as-is
		if (evt.data != lastMsg) {
			lastMsgEl.innerText = evt.data;
			lastMsg = evt.data;
		}
	};
	socket.onopen = function(evt) {
		console.log('Socket opened');
	};

	// Every 1s, ask the Arduino where the servo is
	setInterval(function(e) {
		send(MsgPosition, 0);
	}, 1000);
}

function send(intValue, floatValue) {
	socket.send('<servo,' + intValue + ',' + floatValue + '>\r\n');
}
