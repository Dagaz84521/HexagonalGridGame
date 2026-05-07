(function () {
  "use strict";

  const canvas = document.getElementById("board");
  const ctx = canvas.getContext("2d");
  const hoverBadge = document.getElementById("hoverBadge");
  const distanceBadge = document.getElementById("distanceBadge");
  const coordReadout = document.getElementById("coordReadout");
  const formulaReadout = document.getElementById("formulaReadout");
  const whyReadout = document.getElementById("whyReadout");
  const setAButton = document.getElementById("setAButton");
  const setBButton = document.getElementById("setBButton");
  const resetButton = document.getElementById("resetButton");

  const SQRT3 = Math.sqrt(3);
  const radius = 5;
  const hexSize = 48;
  const dpr = Math.max(1, window.devicePixelRatio || 1);
  const grid = buildHexagon(radius);
  const gridKeys = new Set(grid.map(key));

  let activeTarget = "A";
  let aKey = key({ q: -3, r: 2 });
  let bKey = key({ q: 3, r: -2 });
  let hoverKey = null;

  function key(coord) {
    return `${coord.q},${coord.r}`;
  }

  function parseKey(value) {
    const parts = value.split(",").map(Number);
    return { q: parts[0], r: parts[1] };
  }

  function getS(coord) {
    return -coord.q - coord.r;
  }

  function toCube(coord) {
    return { q: coord.q, r: coord.r, s: getS(coord) };
  }

  function subtractCube(a, b) {
    return {
      q: a.q - b.q,
      r: a.r - b.r,
      s: a.s - b.s
    };
  }

  function distance(a, b) {
    const delta = subtractCube(toCube(a), toCube(b));
    return Math.max(Math.abs(delta.q), Math.abs(delta.r), Math.abs(delta.s));
  }

  function distanceBySum(a, b) {
    const delta = subtractCube(toCube(a), toCube(b));
    return (Math.abs(delta.q) + Math.abs(delta.r) + Math.abs(delta.s)) / 2;
  }

  function sign(value) {
    if (value > 0) {
      return 1;
    }
    if (value < 0) {
      return -1;
    }
    return 0;
  }

  function stepTowards(current, target) {
    const currentCube = toCube(current);
    const targetCube = toCube(target);
    const delta = subtractCube(targetCube, currentCube);
    const nextCube = { ...currentCube };

    if (delta.q !== 0) {
      nextCube.q += sign(delta.q);
      if (delta.r !== 0) {
        nextCube.r += sign(delta.r);
      } else {
        nextCube.s += sign(delta.s);
      }
    } else {
      nextCube.r += sign(delta.r);
      nextCube.s += sign(delta.s);
    }

    return { q: nextCube.q, r: nextCube.r };
  }

  function buildShortestPath(start, target) {
    const result = [key(start)];
    let current = { ...start };
    let safety = 0;

    while (key(current) !== key(target) && safety < 64) {
      current = stepTowards(current, target);
      result.push(key(current));
      safety += 1;
    }

    return result;
  }

  function buildHexagon(maxRadius) {
    const result = [];
    for (let q = -maxRadius; q <= maxRadius; q += 1) {
      const r1 = Math.max(-maxRadius, -q - maxRadius);
      const r2 = Math.min(maxRadius, -q + maxRadius);
      for (let r = r1; r <= r2; r += 1) {
        result.push({ q, r });
      }
    }
    return result;
  }

  function hexToPixel(coord) {
    return {
      x: hexSize * SQRT3 * (coord.q + coord.r / 2),
      y: hexSize * 1.5 * coord.r
    };
  }

  function pixelToHex(point) {
    const q = (SQRT3 / 3 * point.x - 1 / 3 * point.y) / hexSize;
    const r = (2 / 3 * point.y) / hexSize;
    return cubeRound(q, r, -q - r);
  }

  function cubeRound(q, r, s) {
    let rq = Math.round(q);
    let rr = Math.round(r);
    let rs = Math.round(s);
    const qDiff = Math.abs(rq - q);
    const rDiff = Math.abs(rr - r);
    const sDiff = Math.abs(rs - s);

    if (qDiff > rDiff && qDiff > sDiff) {
      rq = -rr - rs;
    } else if (rDiff > sDiff) {
      rr = -rq - rs;
    }

    return { q: rq, r: rr };
  }

  function resize() {
    const rect = canvas.getBoundingClientRect();
    canvas.width = Math.floor(rect.width * dpr);
    canvas.height = Math.floor(rect.height * dpr);
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
  }

  function origin() {
    const rect = canvas.getBoundingClientRect();
    return {
      x: rect.width / 2,
      y: rect.height / 2 + 10
    };
  }

  function screenPoint(coord) {
    const local = hexToPixel(coord);
    const o = origin();
    return { x: o.x + local.x, y: o.y + local.y };
  }

  function corners(center) {
    const points = [];
    for (let index = 0; index < 6; index += 1) {
      const angle = Math.PI / 180 * (60 * index + 30);
      points.push({
        x: center.x + hexSize * Math.cos(angle),
        y: center.y + hexSize * Math.sin(angle)
      });
    }
    return points;
  }

  function drawHex(center, fill, stroke, lineWidth) {
    const pts = corners(center);
    ctx.beginPath();
    pts.forEach((point, index) => {
      if (index === 0) {
        ctx.moveTo(point.x, point.y);
      } else {
        ctx.lineTo(point.x, point.y);
      }
    });
    ctx.closePath();
    ctx.fillStyle = fill;
    ctx.fill();
    ctx.strokeStyle = stroke;
    ctx.lineWidth = lineWidth;
    ctx.stroke();
  }

  function styleForCell(cellKey) {
    let fill = "rgba(148, 163, 184, 0.16)";
    let stroke = "rgba(226, 232, 240, 0.26)";
    let lineWidth = 1.25;
    const a = parseKey(aKey);
    const b = parseKey(bKey);
    const coord = parseKey(cellKey);
    const path = buildShortestPath(a, b);

    if (path.includes(cellKey)) {
      fill = "rgba(250, 204, 21, 0.22)";
      stroke = "rgba(250, 204, 21, 0.72)";
    }

    if (cellKey === aKey) {
      fill = "rgba(74, 222, 128, 0.48)";
      stroke = "rgba(74, 222, 128, 1)";
      lineWidth = 3;
    }

    if (cellKey === bKey) {
      fill = "rgba(96, 165, 250, 0.52)";
      stroke = "rgba(96, 165, 250, 1)";
      lineWidth = 3;
    }

    if (cellKey === hoverKey) {
      stroke = "rgba(255, 255, 255, 1)";
      lineWidth = 3.4;
    }

    return { fill, stroke, lineWidth };
  }

  function drawLineBetweenAAndB() {
    const path = buildShortestPath(parseKey(aKey), parseKey(bKey)).map(parseKey).map(screenPoint);
    if (path.length < 2) {
      return;
    }

    ctx.save();
    ctx.strokeStyle = "rgba(250, 204, 21, 0.72)";
    ctx.lineWidth = 4;
    ctx.lineCap = "round";
    ctx.beginPath();
    path.forEach((point, index) => {
      if (index === 0) {
        ctx.moveTo(point.x, point.y);
      } else {
        ctx.lineTo(point.x, point.y);
      }
    });
    ctx.stroke();
    ctx.restore();
  }

  function draw() {
    const rect = canvas.getBoundingClientRect();
    ctx.clearRect(0, 0, rect.width, rect.height);
    drawLineBetweenAAndB();

    const sorted = grid.slice().sort((left, right) => (left.r - right.r) || (left.q - right.q));
    for (const coord of sorted) {
      const cellKey = key(coord);
      const center = screenPoint(coord);
      const style = styleForCell(cellKey);
      drawHex(center, style.fill, style.stroke, style.lineWidth);

      ctx.textAlign = "center";
      ctx.fillStyle = "rgba(241, 245, 249, 0.94)";
      ctx.font = "11px Consolas, monospace";
      ctx.fillText(`${coord.q},${coord.r}`, center.x, center.y - 14);
      ctx.fillStyle = "rgba(174, 183, 196, 0.84)";
      ctx.font = "10px Consolas, monospace";
      ctx.fillText(`s:${getS(coord)}`, center.x, center.y + 2);
      ctx.fillText(`dA:${distance(parseKey(aKey), coord)}`, center.x, center.y + 17);
    }
  }

  function coordFromEvent(event) {
    const rect = canvas.getBoundingClientRect();
    const o = origin();
    const point = {
      x: event.clientX - rect.left - o.x,
      y: event.clientY - rect.top - o.y
    };
    const coord = pixelToHex(point);
    const cellKey = key(coord);
    return gridKeys.has(cellKey) ? cellKey : null;
  }

  function cubeText(label, coord) {
    const cube = toCube(coord);
    return `${label}  Q:${cube.q} R:${cube.r} S:${cube.s}`;
  }

  function updateReadouts() {
    const a = parseKey(aKey);
    const b = parseKey(bKey);
    const aCube = toCube(a);
    const bCube = toCube(b);
    const delta = subtractCube(aCube, bCube);
    const absQ = Math.abs(delta.q);
    const absR = Math.abs(delta.r);
    const absS = Math.abs(delta.s);
    const maxDistance = Math.max(absQ, absR, absS);
    const sumDistance = (absQ + absR + absS) / 2;
    const path = buildShortestPath(a, b).map(parseKey);

    coordReadout.textContent = [
      cubeText("A", a),
      cubeText("B", b),
      hoverKey ? cubeText("Hover", parseKey(hoverKey)) : "Hover  -",
      "",
      `S = -Q - R`,
      `A.S = ${aCube.s}`,
      `B.S = ${bCube.s}`
    ].join("\n");

    formulaReadout.textContent = [
      "Delta = A - B",
      `dQ = ${aCube.q} - ${bCube.q} = ${delta.q}`,
      `dR = ${aCube.r} - ${bCube.r} = ${delta.r}`,
      `dS = ${aCube.s} - ${bCube.s} = ${delta.s}`,
      "",
      "Distance",
      `max(abs(dQ), abs(dR), abs(dS))`,
      `= max(${absQ}, ${absR}, ${absS})`,
      `= ${maxDistance}`,
      "",
      "Equivalent",
      `(abs(dQ)+abs(dR)+abs(dS))/2`,
      `= (${absQ}+${absR}+${absS})/2`,
      `= ${sumDistance}`
    ].join("\n");

    whyReadout.textContent = buildWhyText(a, b, path, delta, maxDistance, sumDistance);

    hoverBadge.textContent = hoverKey ? `Hover: ${hoverKey}, S ${getS(parseKey(hoverKey))}` : "Hover: -";
    distanceBadge.textContent = `Distance: ${maxDistance}`;
  }

  function buildWhyText(a, b, path, delta, maxDistance, sumDistance) {
    const lines = [
      "1. 每走一步，Cube 坐标一定是：",
      "   一个轴 +1，另一个轴 -1，第三个轴不变。",
      "",
      "2. 因为 Q + R + S 始终等于 0，",
      "   所以 dQ + dR + dS 也始终等于 0。",
      "",
      "3. 每一步最多只能把最大的那个差值削掉 1。",
      `   当前最大差值是 ${maxDistance}，所以至少要 ${maxDistance} 步。`,
      "",
      "4. 同时，因为每步会让两个差值各靠近 0 一格，",
      "   abs(dQ)+abs(dR)+abs(dS) 每步减少 2。",
      `   所以 (${Math.abs(delta.q)}+${Math.abs(delta.r)}+${Math.abs(delta.s)})/2 = ${sumDistance}。`,
      "",
      "5. 下面是一条最短路径，每一行是一格："
    ];

    for (let index = 0; index < path.length; index += 1) {
      const current = path[index];
      const currentDelta = subtractCube(toCube(b), toCube(current));
      lines.push(
        `${String(index).padStart(2, "0")}. ` +
        `(${current.q},${current.r},${getS(current)}) ` +
        `剩余 Δ=(${currentDelta.q},${currentDelta.r},${currentDelta.s}) ` +
        `dist=${distance(current, b)}`
      );
    }

    return lines.join("\n");
  }

  function setMode(mode) {
    activeTarget = mode;
    setAButton.classList.toggle("active", mode === "A");
    setBButton.classList.toggle("active", mode === "B");
  }

  canvas.addEventListener("mousemove", (event) => {
    hoverKey = coordFromEvent(event);
    updateReadouts();
  });

  canvas.addEventListener("mouseleave", () => {
    hoverKey = null;
    updateReadouts();
  });

  canvas.addEventListener("click", (event) => {
    const clicked = coordFromEvent(event);
    if (!clicked) {
      return;
    }

    if (activeTarget === "A" && clicked !== bKey) {
      aKey = clicked;
    } else if (activeTarget === "B" && clicked !== aKey) {
      bKey = clicked;
    }
    updateReadouts();
  });

  setAButton.addEventListener("click", () => setMode("A"));
  setBButton.addEventListener("click", () => setMode("B"));
  resetButton.addEventListener("click", () => {
    aKey = key({ q: -3, r: 2 });
    bKey = key({ q: 3, r: -2 });
    setMode("A");
    updateReadouts();
  });

  window.addEventListener("resize", () => {
    resize();
    draw();
  });

  resize();
  updateReadouts();
  requestAnimationFrame(function frame() {
    draw();
    requestAnimationFrame(frame);
  });
})();
