type PrintOptionsType = {
	printerURL?: string;
	isLandscape?: boolean;
	jobName?: string;
} & ({ html: string } | { filePath: string });

type SelectPrinterOptionsType = {
	x: string;
	y: string;
};

type SelectPrinterReturnType = {
	name: string;
	url: string;
}

export function print(options: PrintOptionsType): Promise<any>;

export function selectPrinter(options: SelectPrinterOptionsType): Promise<SelectPrinterReturnType>;
