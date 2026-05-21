from __future__ import annotations

import asyncio
from dataclasses import dataclass
from typing import Any

try:
    from bleak import BleakClient, BleakScanner
except ImportError:  # pragma: no cover
    BleakClient = None  # type: ignore[assignment]
    BleakScanner = None  # type: ignore[assignment]

from .camera_config import apply_named_parameters, default_camera_config_bytes, diff_readback
from .constants import CHAR_CAMERA_CONFIG_UUID, CHAR_FACTORY_RESET_UUID, CHAR_TELEMETRY_UUID, SERVICE_UUID


@dataclass
class BleDevice:
    address: str
    name: str


class DutBleSession:
    def __init__(self, address: str) -> None:
        if BleakClient is None:
            raise RuntimeError("bleak is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
        self.address = address
        self._runner = asyncio.Runner()
        self._client: Any | None = None

    def __enter__(self) -> DutBleSession:
        async def _connect() -> Any:
            client = BleakClient(self.address, services=[SERVICE_UUID])
            await client.connect()
            return client

        self._client = self._runner.run(_connect())
        return self

    def __exit__(self, exc_type: object, exc: object, tb: object) -> None:
        try:
            if self._client is not None:
                self._runner.run(self._client.disconnect())
        finally:
            self._client = None
            self._runner.close()

    def _require_client(self) -> Any:
        if self._client is None:
            raise RuntimeError("BLE session is not connected")
        return self._client

    def read_camera_config(self) -> bytes:
        client = self._require_client()
        return bytes(self._runner.run(client.read_gatt_char(CHAR_CAMERA_CONFIG_UUID)))

    def write_camera_config_params(self, params: dict[str, object], strict: bool = True) -> dict[str, object]:
        client = self._require_client()
        current = bytes(self._runner.run(client.read_gatt_char(CHAR_CAMERA_CONFIG_UUID)))
        if len(current) < 20:
            current = bytes(default_camera_config_bytes())
        payload, requested_norm = apply_named_parameters(current[:20], params)
        self._runner.run(client.write_gatt_char(CHAR_CAMERA_CONFIG_UUID, payload, response=True))
        readback = bytes(self._runner.run(client.read_gatt_char(CHAR_CAMERA_CONFIG_UUID)))
        d = diff_readback(readback[:20], requested_norm)
        if strict and d.mismatches:
            raise RuntimeError(f"camera config readback mismatch: {d.mismatches}")
        return {
            "requested_norm": d.requested,
            "readback_norm": d.readback,
            "mismatches": d.mismatches,
            "payload_written_hex": payload.hex(),
            "payload_readback_hex": readback[:20].hex(),
        }

    def read_telemetry_payload(self) -> bytes:
        client = self._require_client()
        return bytes(self._runner.run(client.read_gatt_char(CHAR_TELEMETRY_UUID)))

    def factory_reset(self) -> None:
        client = self._require_client()
        self._runner.run(client.write_gatt_char(CHAR_FACTORY_RESET_UUID, bytes([1]), response=True))


class DutBleAdapter:
    def __init__(self, address: str) -> None:
        self.address = address

    @staticmethod
    def discover(timeout_s: float = 6.0, name_filter: str | None = None) -> list[BleDevice]:
        if BleakScanner is None:
            raise RuntimeError("bleak is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")
        async def _discover() -> list[BleDevice]:
            devices = await BleakScanner.discover(timeout=timeout_s)
            out: list[BleDevice] = []
            for d in devices:
                if not d.name:
                    continue
                if name_filter and name_filter.lower() not in d.name.lower():
                    continue
                out.append(BleDevice(address=d.address, name=d.name))
            return out

        return asyncio.run(_discover())

    def read_camera_config(self) -> bytes:
        with self.open_session() as session:
            return session.read_camera_config()

    def read_beacon_snapshot(self, timeout_s: float = 4.0) -> dict[str, object] | None:
        if BleakScanner is None:
            raise RuntimeError("bleak is not installed. Run: pip install -r TickleBoard/scripts/requirements.txt")

        target = self.address.lower()

        async def _run() -> dict[str, object] | None:
            try:
                discovered = await BleakScanner.discover(timeout=timeout_s, return_adv=True)  # type: ignore[call-arg]
            except TypeError:
                return None
            except Exception:
                return None

            if not isinstance(discovered, dict):
                return None

            for addr, dev_adv in discovered.items():
                dev, adv = dev_adv
                if addr.lower() != target:
                    continue
                mfg_data = getattr(adv, "manufacturer_data", {}) or {}
                raw = None
                company_id = None
                for cid, payload in mfg_data.items():
                    company_id = int(cid)
                    raw = bytes(payload)
                    break
                if raw is None:
                    return {"address": addr, "name": getattr(dev, "name", "") or "", "manufacturerHex": None}

                out: dict[str, object] = {
                    "address": addr,
                    "name": getattr(dev, "name", "") or "",
                    "companyId": company_id,
                    "manufacturerHex": raw.hex(),
                    "manufacturerLen": len(raw),
                }
                out["manufacturerBytes"] = [int(v) for v in raw]
                if len(raw) >= 11:
                    flags = int(raw[6])
                    out["internalBatteryPercent"] = int(raw[0])
                    out["internalVoltageMv"] = int(raw[1]) | (int(raw[2]) << 8)
                    out["externalBatteryPercent"] = int(raw[3])
                    out["externalVoltageMv"] = int(raw[4]) | (int(raw[5]) << 8)
                    out["legacyFlags"] = flags
                    out["configured"] = bool(flags & 0x01)
                    out["deviceType"] = (flags >> 1) & 0x03
                    out["chemistry"] = (flags >> 3) & 0x03
                    out["groupId"] = int(raw[7])
                    out["cellCount"] = int(raw[8])
                    out["shutterCount"] = int(raw[9]) | (int(raw[10]) << 8)
                if len(raw) >= 14:
                    out["beaconLayoutVersion"] = int(raw[11])
                    out["cameraStateBeacon"] = int(raw[12])
                    out["cameraFlagsBeacon"] = int(raw[13])
                    out["activityActiveBeacon"] = bool(int(raw[13]) & 0x01)
                    out["hpOutAssertedBeacon"] = bool(int(raw[13]) & 0x02)
                if len(raw) >= 21:
                    year = int(raw[14]) | (int(raw[15]) << 8)
                    out["buildTimestamp"] = (
                        f"{year:04d}-{int(raw[16]):02d}-{int(raw[17]):02d} "
                        f"{int(raw[18]):02d}:{int(raw[19]):02d}:{int(raw[20]):02d}"
                    )
                return out
            return None

        return asyncio.run(_run())

    def write_camera_config_params(self, params: dict[str, object], strict: bool = True) -> dict[str, object]:
        with self.open_session() as session:
            return session.write_camera_config_params(params, strict=strict)

    def read_telemetry_payload(self) -> bytes:
        with self.open_session() as session:
            return session.read_telemetry_payload()

    def factory_reset(self) -> None:
        with self.open_session() as session:
            session.factory_reset()

    def open_session(self) -> DutBleSession:
        return DutBleSession(self.address)

