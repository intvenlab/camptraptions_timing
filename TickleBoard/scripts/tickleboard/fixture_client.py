from __future__ import annotations

from dataclasses import dataclass
import time

try:
    import serial
    import serial.tools.list_ports
except ImportError:  # pragma: no cover
    serial = None  # type: ignore[assignment]


@dataclass
class FixtureIdentity:
    line: str


@dataclass
class EdgeRecord:
    t_ms: int
    signal: str
    state: str


@dataclass
class DumpResult:
    run_id: int | None
    edges: list[EdgeRecord]
    warnings: list[str]
    snapshots: list[str]
    malformed_lines: list[str]


class FixtureClient:
    def __init__(self, port: str, baud: int = 115200, timeout: float = 1.0) -> None:
        if serial is None:
            raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
        self.port = port
        self._ser = serial.Serial(port, baudrate=baud, timeout=timeout)
        time.sleep(1.8)
        self._ser.reset_input_buffer()
        self._ser.reset_output_buffer()

    def close(self) -> None:
        if self._ser and self._ser.is_open:
            self._ser.close()

    @staticmethod
    def list_ports() -> list[tuple[str, str]]:
        if serial is None:
            raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
        return [(p.device, p.description or "") for p in serial.tools.list_ports.comports()]

    @staticmethod
    def pick_default_port() -> str | None:
        if serial is None:
            raise RuntimeError("pyserial is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
        for p in serial.tools.list_ports.comports():
            desc = (p.description or "").lower()
            if "arduino uno" in desc:
                return p.device
        return None

    def _send_line(self, line: str) -> None:
        self._ser.write((line.strip() + "\n").encode("ascii"))
        self._ser.flush()

    def _read_line(self, timeout_s: float = 2.0) -> str | None:
        deadline = time.time() + timeout_s
        while time.time() < deadline:
            raw = self._ser.readline()
            if not raw:
                continue
            txt = raw.decode("ascii", errors="replace").strip()
            if txt:
                return txt
        return None

    def command_ok(self, line: str, retries: int = 1, total_timeout_s: float = 8.0) -> list[str]:
        last: list[str] = []
        for _ in range(retries + 1):
            self._send_line(line)
            out: list[str] = []
            deadline = time.time() + total_timeout_s
            while time.time() < deadline:
                txt = self._read_line(timeout_s=0.8)
                if txt is None:
                    continue
                out.append(txt)
                if txt == "OK" or txt.startswith("ERR ") or txt.startswith("RUN_OK"):
                    break
            last = out
            if out and (out[-1] == "OK" or out[-1].startswith("RUN_OK")):
                return out
        raise RuntimeError(f"fixture command failed: {line!r} -> {last}")

    def identify(self) -> FixtureIdentity:
        self._send_line("ID?")
        txt = self._read_line(timeout_s=2.0)
        if txt is None or not txt.startswith("ID "):
            raise RuntimeError(f"failed to identify fixture on {self.port}")
        return FixtureIdentity(line=txt)

    def dump(self) -> DumpResult:
        self._send_line("DUMP")
        edges: list[EdgeRecord] = []
        warnings: list[str] = []
        snapshots: list[str] = []
        malformed: list[str] = []
        run_id: int | None = None
        started = False
        deadline = time.time() + 10.0

        while time.time() < deadline:
            txt = self._read_line(timeout_s=1.5)
            if txt is None:
                continue
            if txt.startswith("BEGIN LOG"):
                started = True
                if "RUNID=" in txt:
                    try:
                        run_id = int(txt.split("RUNID=")[1].strip())
                    except ValueError:
                        malformed.append(txt)
                continue
            if not started:
                continue
            if txt.startswith("SNAPSHOT "):
                snapshots.append(txt)
                continue
            if txt.startswith("WARN "):
                warnings.append(txt)
                continue
            if txt.startswith("EDGE "):
                parts = txt.split()
                if len(parts) == 4:
                    try:
                        edges.append(EdgeRecord(t_ms=int(parts[1]), signal=parts[2], state=parts[3]))
                        continue
                    except ValueError:
                        pass
                malformed.append(txt)
                continue
            if txt.startswith("END OK"):
                break
            malformed.append(txt)

        return DumpResult(
            run_id=run_id,
            edges=edges,
            warnings=warnings,
            snapshots=snapshots,
            malformed_lines=malformed,
        )

