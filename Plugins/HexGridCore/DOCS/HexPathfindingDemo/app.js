(function () {
  "use strict";

  const {
    DIRECTIONS,
    key,
    parseKey,
    getS,
    distance,
    hexToPixel,
    pixelToHex,
    buildHexagon,
    createAStar
  } = window.HexGridDemo;

  const canvas = document.getElementById("board");
  const ctx = canvas.getContext("2d");
  const hoverBadge = document.getElementById("hoverBadge");
  const pathBadge = document.getElementById("pathBadge");
  const coordReadout = document.getElementById("coordReadout");
  const astarReadout = document.getElementById("astarReadout");
  const modeSelect = document.getElementById("modeSelect");
  const runButton = document.getElementById("runButton");
  const stepButton = document.getElementById("stepButton");
  const cellStepButton = document.getElementById("cellStepButton");
  const resetSearchButton = document.getElementById("resetSearchButton");
  const resetBoardButton = document.getElementById("resetBoardButton");
  const pseudocodeLines = Array.from(document.querySelectorAll("#pseudocode li"));

  const radius = 5;
  const hexSize = 45;
  const dpr = Math.max(1, window.devicePixelRatio || 1);
  const grid = buildHexagon(radius);
  const gridKeys = new Set(grid.map(key));

  let startKey = key({ q: -3, r: 2 });
  let goalKey = key({ q: 3, r: -2 });
  let hoverKey = null;
  let blockedKeys = new Set();
  let search = null;
  let searchState = null;
  let running = false;
  let lastStepAt = 0;

  function resetBlockedKeys() {
    blockedKeys = new Set(["-1,0", "0,0", "1,-1", "1,0", "2,-1", "-2,2"]);
  }

  function createSearch() {
    search = createAStar(gridKeys, blockedKeys, startKey, goalKey);
    searchState = search.snapshot();
    running = false;
    updateReadouts();
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
      y: rect.height / 2 - 8
    };
  }

  function screenPoint(coord) {
    const local = hexToPixel(coord, hexSize);
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

  function drawArrow(from, to, color) {
    const start = screenPoint(parseKey(from));
    const end = screenPoint(parseKey(to));
    const dx = end.x - start.x;
    const dy = end.y - start.y;
    const len = Math.hypot(dx, dy);
    if (len < 1) {
      return;
    }
    const ux = dx / len;
    const uy = dy / len;
    const a = { x: start.x + ux * 18, y: start.y + uy * 18 };
    const b = { x: end.x - ux * 18, y: end.y - uy * 18 };

    ctx.strokeStyle = color;
    ctx.lineWidth = 4;
    ctx.lineCap = "round";
    ctx.beginPath();
    ctx.moveTo(a.x, a.y);
    ctx.lineTo(b.x, b.y);
    ctx.stroke();
    ctx.lineCap = "butt";
  }

  function drawCameFromArrow(fromKey, toKey) {
    const start = screenPoint(parseKey(fromKey));
    const end = screenPoint(parseKey(toKey));
    const dx = end.x - start.x;
    const dy = end.y - start.y;
    const len = Math.hypot(dx, dy);
    if (len < 1) {
      return;
    }

    const ux = dx / len;
    const uy = dy / len;
    const startOffset = 11;
    const endOffset = 16;
    const a = { x: start.x + ux * startOffset, y: start.y + uy * startOffset };
    const b = { x: end.x - ux * endOffset, y: end.y - uy * endOffset };
    const left = {
      x: b.x - ux * 6 - uy * 4,
      y: b.y - uy * 6 + ux * 4
    };
    const right = {
      x: b.x - ux * 6 + uy * 4,
      y: b.y - uy * 6 - ux * 4
    };

    ctx.strokeStyle = "rgba(250, 204, 21, 0.34)";
    ctx.fillStyle = "rgba(250, 204, 21, 0.52)";
    ctx.lineWidth = 1.6;
    ctx.beginPath();
    ctx.moveTo(a.x, a.y);
    ctx.lineTo(b.x, b.y);
    ctx.stroke();
    ctx.beginPath();
    ctx.moveTo(b.x, b.y);
    ctx.lineTo(left.x, left.y);
    ctx.lineTo(right.x, right.y);
    ctx.closePath();
    ctx.fill();
  }

  function styleForCell(cellKey) {
    let fill = "rgba(148, 163, 184, 0.16)";
    let stroke = "rgba(226, 232, 240, 0.26)";
    let lineWidth = 1.25;
    const coord = parseKey(cellKey);

    if (searchState && searchState.closed.has(cellKey)) {
      fill = "rgba(167, 139, 250, 0.27)";
      stroke = "rgba(167, 139, 250, 0.9)";
    }
    if (searchState && searchState.open.has(cellKey)) {
      fill = "rgba(34, 211, 238, 0.28)";
      stroke = "rgba(34, 211, 238, 0.9)";
    }
    if (searchState && searchState.path.includes(cellKey)) {
      fill = "rgba(250, 204, 21, 0.48)";
      stroke = "rgba(250, 204, 21, 1)";
      lineWidth = 2.4;
    }
    if (blockedKeys.has(cellKey)) {
      fill = "rgba(251, 113, 133, 0.45)";
      stroke = "rgba(251, 113, 133, 1)";
      lineWidth = 2.2;
    }
    if (cellKey === startKey) {
      fill = "rgba(74, 222, 128, 0.48)";
      stroke = "rgba(74, 222, 128, 1)";
      lineWidth = 2.6;
    }
    if (cellKey === goalKey) {
      fill = "rgba(96, 165, 250, 0.5)";
      stroke = "rgba(96, 165, 250, 1)";
      lineWidth = 2.6;
    }
    if (searchState && cellKey === searchState.currentKey && cellKey !== goalKey && cellKey !== startKey) {
      stroke = "rgba(250, 204, 21, 1)";
      lineWidth = 3.2;
    }
    if (searchState && cellKey === searchState.currentNeighborKey && cellKey !== goalKey && cellKey !== startKey) {
      stroke = "rgba(255, 255, 255, 1)";
      lineWidth = 3.4;
    }
    if (cellKey === hoverKey) {
      stroke = "rgba(255, 255, 255, 1)";
      lineWidth = 3.2;
    }

    return { fill, stroke, lineWidth };
  }

  function draw() {
    const rect = canvas.getBoundingClientRect();
    ctx.clearRect(0, 0, rect.width, rect.height);

    if (searchState && searchState.path.length > 0) {
      let previous = startKey;
      for (const next of searchState.path) {
        drawArrow(previous, next, "rgba(250, 204, 21, 0.7)");
        previous = next;
      }
    }

    if (searchState && searchState.cameFrom) {
      for (const [toKey, fromKey] of searchState.cameFrom) {
        drawCameFromArrow(fromKey, toKey);
      }
    }

    const sorted = grid.slice().sort((a, b) => (a.r - b.r) || (a.q - b.q));
    for (const coord of sorted) {
      const cellKey = key(coord);
      const center = screenPoint(coord);
      const style = styleForCell(cellKey);
      drawHex(center, style.fill, style.stroke, style.lineWidth);

      ctx.textAlign = "center";
      ctx.fillStyle = "rgba(241, 245, 249, 0.92)";
      ctx.font = "11px Consolas, monospace";
      ctx.fillText(`${coord.q},${coord.r}`, center.x, center.y - 22);

      const g = searchState && searchState.gCost ? searchState.gCost.get(cellKey) : undefined;
      const h = distance(coord, parseKey(goalKey));
      const f = searchState && searchState.fCost ? searchState.fCost.get(cellKey) : undefined;
      const from = searchState && searchState.cameFrom ? searchState.cameFrom.get(cellKey) : undefined;

      ctx.fillStyle = "rgba(226, 232, 240, 0.9)";
      ctx.font = "10px Consolas, monospace";
      ctx.fillText(
        `G:${g === undefined ? "-" : g} H:${h} F:${f === undefined ? "-" : f}`,
        center.x,
        center.y - 4
      );
      ctx.fillStyle = "rgba(174, 183, 196, 0.82)";
      ctx.fillText(`from:${from ? from : "-"}`, center.x, center.y + 13);
    }

  }

  function coordFromEvent(event) {
    const rect = canvas.getBoundingClientRect();
    const o = origin();
    const point = {
      x: event.clientX - rect.left - o.x,
      y: event.clientY - rect.top - o.y
    };
    const coord = pixelToHex(point, hexSize);
    const cellKey = key(coord);
    return gridKeys.has(cellKey) ? cellKey : null;
  }

  function formatCoord(label, value) {
    if (!value) {
      return `${label.padEnd(8)}-`;
    }
    const coord = parseKey(value);
    return `${label.padEnd(8)}Q:${coord.q} R:${coord.r} S:${getS(coord)}`;
  }

  function updateReadouts() {
    const start = parseKey(startKey);
    const goal = parseKey(goalKey);
    const neighbourLines = DIRECTIONS
      .map((direction) => `${direction.name.padEnd(12)}(${direction.q}, ${direction.r})`)
      .join("\n");

    const hoverDetail = getCellDebugText(hoverKey);

    coordReadout.textContent = [
      formatCoord("Hover", hoverKey),
      formatCoord("Start", startKey),
      formatCoord("Goal", goalKey),
      `Distance ${distance(start, goal)}`,
      "",
      "Hover Detail",
      hoverDetail,
      "",
      "Direction Offsets",
      neighbourLines
    ].join("\n");

    const pathText = searchState && searchState.path.length > 0
      ? searchState.path.join(" -> ")
      : "-";
    const currentText = searchState && searchState.currentKey ? searchState.currentKey : "-";
    const neighborText = searchState && searchState.currentNeighborKey ? searchState.currentNeighborKey : "-";
    const openText = searchState ? searchState.open.size : 0;
    const closedText = searchState ? searchState.closed.size : 0;
    const statusText = searchState && searchState.finished
      ? (searchState.found ? "finished: found" : "finished: no path")
      : "searching / ready";

    astarReadout.textContent = [
      `Status  ${statusText}`,
      `Step    ${searchState ? searchState.message : "-"}`,
      `Current ${currentText}`,
      `Neighbor ${neighborText}`,
      `Open    ${openText}`,
      `Closed  ${closedText}`,
      `Path    ${pathText}`
    ].join("\n");

    updatePseudocode();

    if (hoverKey) {
      const coord = parseKey(hoverKey);
      hoverBadge.textContent = `Hover: Q ${coord.q}, R ${coord.r}, S ${getS(coord)}`;
    } else {
      hoverBadge.textContent = "Hover: -";
    }
    pathBadge.textContent = searchState && searchState.path.length > 0
      ? `Path: ${searchState.path.length} steps`
      : "Path: -";
  }

  function getCellDebugText(cellKey) {
    if (!cellKey || !searchState) {
      return "-";
    }

    const coord = parseKey(cellKey);
    const g = searchState.gCost ? searchState.gCost.get(cellKey) : undefined;
    const h = distance(coord, parseKey(goalKey));
    const f = searchState.fCost ? searchState.fCost.get(cellKey) : undefined;
    const from = searchState.cameFrom ? searchState.cameFrom.get(cellKey) : undefined;
    const state = [
      cellKey === startKey ? "start" : "",
      cellKey === goalKey ? "goal" : "",
      blockedKeys.has(cellKey) ? "blocked" : "",
      searchState.open.has(cellKey) ? "open" : "",
      searchState.closed.has(cellKey) ? "closed" : "",
      searchState.path.includes(cellKey) ? "path" : ""
    ].filter(Boolean).join(", ") || "unvisited";

    return [
      `State    ${state}`,
      `GCost    ${g === undefined ? "-" : g}`,
      `HCost    ${h}`,
      `FCost    ${f === undefined ? "-" : f}`,
      `CameFrom ${from || "-"}`
    ].join("\n");
  }

  function updatePseudocode() {
    const activeLine = searchState ? String(searchState.activeLine) : "1";
    for (const line of pseudocodeLines) {
      const lineNumber = line.dataset.line;
      line.classList.toggle("active", lineNumber === activeLine);
      line.classList.toggle("done", Number(lineNumber) < Number(activeLine));
    }
  }

  function stepSearch() {
    if (!search || searchState.finished) {
      return;
    }
    searchState = search.step();
    updateReadouts();
  }

  function stepOneCellSearch() {
    if (!search || searchState.finished) {
      return;
    }

    const closedBefore = searchState.closed.size;
    let expandedCell = false;
    let safety = 0;

    while (!searchState.finished && safety < 160) {
      stepSearch();
      safety += 1;

      if (searchState.closed.size > closedBefore) {
        expandedCell = true;
      }

      // Stop after one current cell has completed all six-neighbour checks.
      if (expandedCell && searchState.activeLine === 3) {
        break;
      }
    }
  }

  function frame(now) {
    if (running && now - lastStepAt > 360) {
      stepSearch();
      lastStepAt = now;
      if (searchState.finished) {
        running = false;
        runButton.textContent = "播放 A*";
      }
    }
    draw();
    requestAnimationFrame(frame);
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

    const mode = modeSelect.value;
    if (mode === "start" && clicked !== goalKey && !blockedKeys.has(clicked)) {
      startKey = clicked;
    } else if (mode === "goal" && clicked !== startKey && !blockedKeys.has(clicked)) {
      goalKey = clicked;
    } else if (mode === "block" && clicked !== startKey && clicked !== goalKey) {
      if (blockedKeys.has(clicked)) {
        blockedKeys.delete(clicked);
      } else {
        blockedKeys.add(clicked);
      }
    }

    createSearch();
  });

  runButton.addEventListener("click", () => {
    if (searchState.finished) {
      createSearch();
    }
    running = !running;
    lastStepAt = 0;
    runButton.textContent = running ? "暂停" : "播放 A*";
  });

  stepButton.addEventListener("click", () => {
    running = false;
    runButton.textContent = "播放 A*";
    stepSearch();
  });

  cellStepButton.addEventListener("click", () => {
    running = false;
    runButton.textContent = "播放 A*";
    stepOneCellSearch();
  });

  resetSearchButton.addEventListener("click", () => {
    createSearch();
  });

  resetBoardButton.addEventListener("click", () => {
    startKey = key({ q: -3, r: 2 });
    goalKey = key({ q: 3, r: -2 });
    resetBlockedKeys();
    createSearch();
  });

  window.addEventListener("resize", () => {
    resize();
    draw();
  });

  resetBlockedKeys();
  resize();
  createSearch();
  requestAnimationFrame(frame);
})();
