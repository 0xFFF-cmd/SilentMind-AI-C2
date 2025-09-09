# decision.py
from flask import Blueprint, request, jsonify
from server.llm import get_llm_decision
import json
from colorama import Fore, Style, init

init(autoreset=True)

decision_bp = Blueprint("decision", __name__)

@decision_bp.route("/decision", methods=["POST"])
def decide():
    print(f"{Fore.CYAN}== [DECISION API HIT] ==")
    print(f"{Fore.YELLOW}HEADERS:{Style.RESET_ALL}", request.headers)
    print(f"{Fore.YELLOW}RAW BODY:{Style.RESET_ALL}", request.get_data())

    try:
        data = request.get_json(force=True)
    except Exception as e:
        print(f"{Fore.RED}[❌] Failed to parse JSON:{Style.RESET_ALL} {e}")
        print(f"{Fore.YELLOW}[RAW BODY]:{Style.RESET_ALL} {request.data.decode(errors='replace')}")
        return jsonify({"error": "Malformed JSON", "details": str(e)}), 400
    
    if not data:
        print(f"{Fore.RED}[❌] Empty or invalid JSON payload after parsing.{Style.RESET_ALL}")
        return jsonify({"error": "Invalid JSON payload"}), 400


    fingerprint = data.get("fingerprint")
    file_list = data.get("files")

    if not fingerprint or not file_list:
        print(f"{Fore.RED}[❌] Missing fingerprint or files{Style.RESET_ALL}")
        return jsonify({"error": "Missing fingerprint or files"}), 400

    print(f"{Fore.MAGENTA}RAW INPUT:{Style.RESET_ALL}", data)
    print(f"{Fore.CYAN}FINGERPRINT:{Style.RESET_ALL}", fingerprint)
    print(f"{Fore.CYAN}FILES:{Style.RESET_ALL}", file_list)

    decision = get_llm_decision(fingerprint, file_list)
    print(f"{Fore.GREEN}[LLM DECISION RAW]:{Style.RESET_ALL}", decision)

    try:
        decision_json = json.loads(json.dumps(decision))  # handles dict already
        return jsonify(decision_json), 200
    except Exception as e:
        print(f"{Fore.RED}[❌] Failed to parse LLM output:{Style.RESET_ALL}", e)
        return jsonify({"error": "LLM response could not be parsed", "raw": decision}), 500